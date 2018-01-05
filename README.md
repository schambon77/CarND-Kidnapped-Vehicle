# Kidnapped Vehicle Project

## Project Objectives
Our robot has been kidnapped and transported to a new location! Luckily it has a map of this location, a (noisy) GPS estimate of its initial location, and lots of (noisy) sensor and control data.

In this project we implement a 2 dimensional particle filter in C++. Our particle filter is given a map and some initial localization information (analogous to what a GPS would provide). At each time step our filter gets observation and control data. 

## Project Files
* [Source code](https://github.com/schambon77/CarND-Kidnapped-Vehicle/tree/master/src)
* [ReadMe](https://github.com/schambon77/CarND-Kidnapped-Vehicle/blob/master/README.md)

## Project Rubric Points

### Accuracy

Does your particle filter localize the vehicle to within the desired accuracy?

This criteria is met as we get to the output saying "Success! Your particle filter passed!".

### Performance

Does your particle run within the specified time of 100 seconds?

This criteria is met as we get to the output saying "Success! Your particle filter passed!".

### General

Does your code use a particle filter to localize the robot?

Our code implement a particle filter as taught in the video lessons.
