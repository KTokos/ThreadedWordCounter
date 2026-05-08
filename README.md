# Threaded Word Counter

**Project Description**

Simply a sample project to help me learn multithreading within C++

Uses 5 threads:

  - One for looking through the "TextFiles" directory and adding each file path to a queue
  - Two for processing the files found within "TextFiles"
  - One for looping through the queue for the word count and popping/adding each number to the total count
