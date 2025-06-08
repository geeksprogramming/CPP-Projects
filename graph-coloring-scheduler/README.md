## Exam Scheduler (Graph Coloring)

This project implements an exam scheduling system using graph coloring. Given data on courses, students enrolled in those courses, and available exam timeslots, the program assigns each course to a timeslot such that no student has overlapping exams. It models the problem as a graph where each course is a node and an edge connects any two courses that share at least one student.

The goal is to generate a valid schedule that minimizes timeslot usage while avoiding exam conflicts. It includes data cleaning to ensure consistency between course and student records and uses an efficient graph coloring strategy to find a feasible schedule within the given constraints.
