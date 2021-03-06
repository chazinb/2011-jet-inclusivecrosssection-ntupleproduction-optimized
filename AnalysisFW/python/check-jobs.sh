#!/bin/bash


# How to use check-jobs.sh
#-------------------------------------------------------------------------------
if [ $# -lt 1 ]; then
    echo " "
    echo " Check the status of the jobs"
    echo " "
    echo "   ./check-jobs.sh 0"
    echo " "
    echo " Print the list of failed files and creates samples_to_resubmit.txt on your current directory"
    echo " "
    echo " Remove the bad jobs"
    echo " "
    echo "   ./check-jobs.sh 1"
    echo " "
    echo " Move the bad jobs to a new folder Badjobs"
    echo " "
    echo "   ./check-jobs.sh 2"
    echo " "
    exit -1
fi

export OPTION=$1

export NPEND=`bjobs | grep PEND | wc -l`
export NRUN=`bjobs | grep RUN | wc -l`

export NFINISH=0
export NGOOD=0

#if [ -d "jobs" ]; then
    NFINISH=`ls LSFJOB_*/STDOUT | wc -l`
    NGOOD=`cat LSFJOB_*/STDOUT | grep T---Report | wc -l`
#fi


# Check the status of the jobs
#-------------------------------------------------------------------------------
printf " \n"
printf " %3d jobs pending\n"    $NPEND
printf " %3d jobs running\n"    $NRUN
printf " %3d jobs finished\n"   $NFINISH
printf " %3d jobs successful\n" $NGOOD
printf " \n"

if [ $NGOOD -ne $NFINISH ]; then

    printf " The following samples have failed:" 

    for fn in `ls LSFJOB_*/STDOUT`; do

	export ISDONE=`cat $fn | grep T---Report | wc -l`
	
	if [ $ISDONE -ne 1 ]; then

	    echo " "
	    printf " %s\n" $fn
	    echo " "
            
	    export FILENAME=`cat $fn | grep  'eospublic.cern.ch'`
            
            echo  $FILENAME >> _mid_samples_to_resubmit.txt
            echo $FILENAME
	    echo " "
           
	    if [ "$OPTION" -eq "1" ]; then

              rm -rf $fn
	
	   fi
	fi
    done
    
    sed  's/filename: //g' _mid_samples_to_resubmit.txt > samples_to_resubmit.txt
    rm   _mid_samples_to_resubmit.txt
 
    if [ "$OPTION" -eq "2" ]; then

        mkdir -p BadJobs
        mv `find $(grep -L T---Report  LSFJOB_*/STDOUT ) -name STDOUT  -printf '%h\n'` BadJobs/.
  
    fi     
    printf " \n"
fi
