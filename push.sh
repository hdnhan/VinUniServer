#!/bin/bash

# general
echo '**/main.dSYM' >.gitignore
echo '**/.vscode' >>.gitignore
echo '**/.ipynb_checkpoints' >>.gitignore
echo '**/__pycache__' >>.gitignore
echo '**/.DS_Store' >>.gitignore

echo '' >>.gitignore
echo '# ignore files greater than 100M' >>.gitignore
# linux
find * -size +100M | cat >>.gitignore
# windows
# forfiles /s /c "cmd /q /c if @fsize GTR 100000000 echo @relpath" >> .gitignore

# set time zone
# echo "$LC_TIME"
LC_TIME=en_US.utf8
# echo "$LC_TIME"

git config user.name "hdnhan"
git config user.email "hdnhan28@gmail.com"
git config user.name
git config user.email

# add all untracked files
git add .
# commit with message datetime if message is null
if [ -z "$1" ]; then
    # get current datetime in +7 timezone
    date=$(TZ="UTC-7" date)
    git commit -m "$date"
else
    # * means the whole arguments/string
    git commit -m "$*"
fi
git push -u origin master
# git push
