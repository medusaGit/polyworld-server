#! /bin/bash

# This script creates a set of symbolic links
# organizing a collection of driven and passive
# Polyworld runs, generated by using the pwfarm
# scripts, so they may be used by the plot script.

function usage()
{
	echo "usage: $0 farm_dir "
	echo
	echo "	farm_dir: name of the specific experimental results"
	echo "		E.g., alifexi2"
	echo
	echo " 	This script must be run from within the polyworld-farm-results directory"
	echo "	containing the specified farm_dir, and farm_dir must contain \"driven\""
	echo "	and \"passive\" sub-directories, each containing a set of run_# directories."
	echo "	Upon successful completion of this script, a new directory will exist,"
	echo "	named the same as farm_dir with \"dp\" appended, which will contain a"
	echo "	set of run_<driven|passive>_# symbolic links to the original run_# directories."
	
	if [ ! -z "$1" ]; then
		err $*
	fi
	
	exit 1
}

function err()
{
    echo "$*">&2
    exit 1
}

if [[ -z "$1" ]]; then
    usage
fi

mkdir -p $1dp

for dir in $1/*
do
	dp=`echo $dir | sed "s/.*\///g"`
	if [[ $dp == "driven" || $dp == "passive" ]]; then
		for file in $dir/*
		do
			ln -sf ../$file "$1dp/run_"$dp"_`echo $file | sed \"s/.*_//g\"`"
		done
	fi
done
