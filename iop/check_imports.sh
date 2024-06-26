# Import checked by El_isra. this script parses a IRX module source code checking for ocurrences of every importted
# function that was defined on imports.lst
# ensuring that you are not wasting binary size on functions that youre not using
RED='\033[1;31m'
YELLOW='\033[1;33m'
GREEN='\033[1;32m'
NC='\033[0m' # No Color
RETCODE=0
IOP_SRC_DIR=src/

for i in $@
do 
  if [ "$i" = "--help" ]; then
	  echo "available switches:"
	  echo "--iop-src-dir=DIR : specify an alternative dir for parsing. by default it is 'src'. but this should be the makefile IOP_SRC_DIR"
	  exit
  fi
  if [[ "$i" == --iop-src-dir=* ]]; then
	  IOP_SRC_DIR=${i#"--iop-src-dir="}
    echo $IOP_SRC_DIR
  fi
done

if [ -f "$IOP_SRC_DIR/imports.lst" ]; then
  
  IMPORTS=$(grep -Eo "^I_[a-zA-Z0-9_-]*" $IOP_SRC_DIR/imports.lst | sed 's/^I_//')
  #echo $IMPORTS
  U=0
  for a in $IMPORTS
  do
      grep -r -q --include="*.c" --include="*.h" "$a" "src/"
      if [ $? -eq 1 ];
      then
          printf "${RED}-- IMPORT ${YELLOW}$a${RED} NOT FOUND IN C SOURCE${NC}\n"
          U=1
      fi
  
  done
  if [ $U -eq 0 ];
  then
      printf "${GREEN}--- ALL IMPORTS USED AT LEAST ONCE${NC}\n"
  fi
else
  echo - 'imports.lst' not found
fi

exit $RETCODE