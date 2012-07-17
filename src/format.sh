#!/bin/bash

for i in `find . -name "*.h" -o -name "*.cpp"`
do
	bcpp ${i} > ${i}2;
	mv ${i}2 ${i}
done;

