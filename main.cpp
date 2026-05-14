// Includes
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

// Constants
const string TEXTFILEDIRECTORY = "/home/ktokos05/Stuff/Outside Projects/Projects/C++/CLion/ThreadedWordCounter/TextFiles";

// Needed variables
queue<string> fileQueue;    // Holds files to be processed by later threads
queue<int> wordCounts;      // Holds each word count from the files processed
mutex mtx;                  // Ensures only one thread can use the same resource at a time
condition_variable cv_allFilesFound;    // Pager to wake up processing threads
condition_variable cv_filesProcessed;   // Pager to wake up final thread
int totalWordCount = 0;     // Holds total word count, used by final thread

// Function for 1 producer thread
void findFiles()
{
    // Check if the directory exists
    if (exists(TEXTFILEDIRECTORY) && is_directory(TEXTFILEDIRECTORY)) {
        // Loop through each item in the directory
        for (const auto& entry : directory_iterator(TEXTFILEDIRECTORY)) {
            // Check if the file is a text file
            if (entry.path().extension() == ".txt") {
                // Add the file path to the fileQueue
                fileQueue.push(entry.path().string());
            }
        }
    }

    // Ring the pager to wake up the processing threads
    cv_allFilesFound.notify_all();
}

// Function for multiple consumer/producer threads
void readFile()
{
    // Lock the mutex so only one thread can access the queue at a time
    unique_lock<mutex> lock(mtx);

    // Wait until the first thread pages us that all files have been found
    cv_allFilesFound.wait(lock);

    // Process a file only if there is actually a file within the queue
    while (fileQueue.empty() == false)
    {
        // Get file name and pop from queue
        string fileName = fileQueue.front();
        fileQueue.pop();

        // Read word by word as long as file is open
        if (ifstream readFile(fileName); readFile.is_open())
        {
            // Create a count to be added to the queue after processing
            int tempCount = 0;

            // Create a string to hold each word
            string word;

            while (readFile >> word)
            {
                tempCount++;
            }

            // Push the count to the queue
            wordCounts.push(tempCount);
        }
    }

    // Step away from the shared queue so other threads can use it
    lock.unlock();

    // Ring the pager to wake up the last thread only if there are no more files in the queue
    if (fileQueue.empty() == true)
    {
        cv_filesProcessed.notify_all();
    }
}

// Function for final thread
void addUpCounts()
{
    // Grab the lock, wait until thread is ready to be woken
    unique_lock<mutex> lock(mtx);

    // Wake up thread once all files are processed
    cv_filesProcessed.wait(lock);

    // Loop through queue of word counts
    while (wordCounts.empty() == false)
    {
        // Get next count and pop from queue
        int currentCount = wordCounts.front();
        wordCounts.pop();

        // Add to total
        totalWordCount += currentCount;
    }

    // Unlock thread once finished
    lock.unlock();
}

int main()
{
    // Create a thread for workers
    vector<thread> workers;

    // Create one thread for finding files
    workers.emplace_back(findFiles);

    // Create one worker for processing files
    workers.emplace_back(readFile);

    // Create one worker for adding counts
    workers.emplace_back(addUpCounts);

    // Wait for the worker threads to finish
    for (auto& t : workers) t.join();

    // Output total count
    cout << totalWordCount << endl;
}
