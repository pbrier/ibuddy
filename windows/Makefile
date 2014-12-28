###########################################
# Simple Makefile for HIDAPI ibuddy program
#
# Peter Brier
# KekkeTek
# 2014-12-28
###########################################

all: ibuddy

CC=gcc
CXX=g++
COBJS=hid.o
CPPOBJS=  ../ibuddy/ibuddy.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS=-I../hidapi -g -c 
LIBS= -lsetupapi

ibuddy: ../ibuddy/ibuddy.o hid.o
	g++ -g $^ $(LIBS) --static -o ibuddy    
    
  $(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm *.o ../ibuddy/*.o ../ibuddy/*.exe

.PHONY: clean
