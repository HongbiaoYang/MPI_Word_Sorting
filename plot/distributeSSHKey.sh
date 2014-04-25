#!/bin/bash
if [ "$1" == "" ];then  
echo "---------------------------------------------------"
echo "---------SSH Key Distribution Script---------------"
echo "---------------------------------------------------"
echo "This script appends your public key ( ~/.ssh/id_rsa )"
echo " to the authrized_key file of your destination"
echo 
echo "Usage: distributeSSHKey username@host";
  exit 1
fi


#----------------
FILE=~/.ssh/id_rsa.pub
if [ ! -f $FILE ];
then
  echo "Public key does not exist.  "
  read -p "Would you like to create one [Y/N]?" -n 1 -r -t 25
  echo ""

  if [[ ! $REPLY =~ ^[Yy]$ ]]
  then
	exit 1
  fi
  echo "calling :"
  echo "ssh-keygen -t rsa"
  ssh-keygen -t rsa
  
  if [ ! -f $FILE ];
  then
    echo "Public key not generated. Exiting"
  fi
fi

ssh $1 "
	cat > /tmp/key ; 
	mkdir ~/.ssh;
	chmod 700 ~/.ssh; 
	cat /tmp/key >> ~/.ssh/authorized_keys;
	chmod 700 ~/.ssh/authorized_keys; 
	rm /tmp/key 
" <$FILE > /dev/null 2>/dev/null

