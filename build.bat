echo Building simonezini97/fba-pomdp:build

docker build -t simonezini97/fba-pomdp:build . -f Dockerfile.build

docker container create --name extract simonezini97/fba-pomdp:build  
docker container cp extract:/fba-pomdp/planning ./
docker container cp extract:/fba-pomdp/bapomdp ./
docker container cp extract:/fba-pomdp/fbapomdp ./
docker container rm -f extract

echo Building simonezini97/fba-pomdp:latest

docker build --no-cache -t simonezini97/fba-pomdp:latest .
del planning /Q
del bapomdp /Q
del fbapomdp /Q
