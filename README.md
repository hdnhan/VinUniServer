# SSH Public Key

## Create a new key pair
* Open terminal on Linux/macOS or PowerShell on Windows, type:
```
ssh-keygen -t ed25519
```
* Then tap 'Enter' four times.

Expected result on Windows:

<img width="581" alt="image" src="https://user-images.githubusercontent.com/22287261/173214087-57db2dd9-59f6-45fc-824a-db565c1eeec8.png">


## Show your public key
* On Linux/macOS, type on terminal:
```
cat ~/.ssh/id_ed25519.pub
```

* On Windows, type on PowerShell:
```
cat $HOME/.ssh/id_ed25519.pub
```
<img width="790" alt="image" src="https://user-images.githubusercontent.com/22287261/173214118-26bde204-f48b-4cc4-8928-0f5ba25bcd7d.png">

# Create config file
* On Linux/macOS, open terminal:
```
nano ~/.ssh/config
```
* On Windows, open PowerShell:
```
echo >> $HOME/.ssh/config
```

Copy and paste this block
```
Host server
    HostName 18.141.200.25
    User ubuntu
    Port [port]
```
<img width="379" alt="image" src="https://user-images.githubusercontent.com/22287261/173214236-afab63a4-b14c-4e1f-a1a7-8948d417e0f8.png">

**Note**: PM me to get your own *port*.

# Connect
```
ssh server
```
