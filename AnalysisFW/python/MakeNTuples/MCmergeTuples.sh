
# Helper script to merge tuples 

cwd=$(pwd)

# Environment
echo "CMSSW environment"
cd /afs/cern.ch/user/b/bchazinq/work/CMSSW_5_3_32
eval `scramv1 runtime -sh`
cd -

echo "EOS environment"
source /afs/cern.ch/project/eos/installation/cms/etc/setup.sh

## Mount EOS
##mkdir eos
##eosmount eos/
##cd eos/cms/store/group/phys_smp/mhaapale/Jet/crab_OpenDataTree_all/160808_155332/0000


# Top-level directory of the N-tuples outputs
cd /eos/user/b/bchazinq/

# Input directories and files
DIRS="QCDPt*to*/"
REGEX="QCDPt([[:digit:]]+to[[:digit:]]+)/"
PREFIX="output_"


# Loop over directories
for d in $DIRS
do
    if [[ ${d} =~ ${REGEX} ]]
    then
        echo "${BASH_REMATCH[1]}"
        # Filenames
        FILES=$d/${PREFIX}*.root

        # Number of files
        NUM=$(ls -l $FILES | wc -l)
        echo "Merging $NUM files..." 

        # Merging 
        hadd -f $d/MCtuples_${REGEX}.root $FILES &
    fi
done




cd /eos/user/b/bchazinq/QCDPt30to50
cd /eos/user/b/bchazinq/QCDPt50to80
cd /eos/user/b/bchazinq/QCDPt80to120
cd /eos/user/b/bchazinq/QCDPt120to170
cd /eos/user/b/bchazinq/QCDPt170to300
cd /eos/user/b/bchazinq/QCDPt300to470

# Merge files
MC=output_*.root
#DATA=OpenDataTree_data_*.root
NUM=$(ls -l $MC | wc -l)
echo "Merging $NUM files..." 

hadd -f tuples_MC30-50.root $MC
hadd -f tuples_MC50-80.root $MC
hadd -f tuples_MC80-120.root $MC
hadd -f tuples_MC120-170.root $MC
hadd -f tuples_MC170-300.root $MC
hadd -f tuples_MC300-470.root $MC

cd $cwd
#eosumount eos/
