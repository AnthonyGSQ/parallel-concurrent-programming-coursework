include ../../common/Makefile

ARGS=./jobs/job001b/job001.txt
INCLUDE=$(DIRS:%=-I%) -Iinclude
FLAGS=-fopenmp
CC=mpicc
XC=mpic++
RUNPRE=mpiexec -np 4