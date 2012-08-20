#!/bin/sh

# {-p 29418, -T}
PORT=-T
export SERVER_NAME="git@github.com"
KEYWEB="https://github.com/account/ssh"

if [ -z ${SERVER_NAME} ]; then
    echo "need env : SERVER_NAME"
    exit 0;
fi


if [ ! -f ~/.ssh/id_rsa.pub ]; then
    ssh-keygen -t rsa
fi

cat ~/.ssh/id_rsa.pub 
echo "[ copy ssh-rsa public key to ${KEYWEB} ]"

sleep 1
read -p 'Press any key to continue...' tmp

export SSH_AUTH_SOCK=""
ssh ${PORT} ${SERVER_NAME}
echo "[ connect to ${SERVER_NAME}, and made know_hosts: ]"
ls -l ~/.ssh/known_hosts

# set the default ssh id to your Gerrit ID 
cat > ~/.ssh/config <<EOF
User gqiao
EOF

echo "TODO: curl http://android.git.kernel.org/repo > /bin/repo"

cat > ~/.gitconfig <<EOF
[user]
        name = George Qiao
        email = joesensport@gmail.com
[github]
        user = gqiao

EOF

#token = 35a0c89ac6b7e8328409fa3d24bd5543

#github_repo




