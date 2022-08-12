# DOCKER
## Build an image
```bash
cd docker/image
docker build -t ubuntu:v22.04 .
```

## Create a container
```bash
cd docker
./create_env.sh
# then enter text or choose otpion
```
# MONITOR 
TODO: Need to test more to catch exceptions
```bash
cd monitor
make # re-compile

watch -n1 ./smi # normal mode
watch -n1 -c ./smi -c # colorize texts
watch -n1 -c ./msi -cd # above && get docker container name of each pid
```

Expected output:

<img width="652" alt="Screen Shot 2022-08-02 at 2 51 16 AM" src="https://user-images.githubusercontent.com/22287261/182234674-8da278d1-0dc9-4aae-8e5a-49cd2604e26f.png">
