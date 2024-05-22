docker build -f docker/base.Dockerfile -t fortnite-gamers:base . &&
docker build -f docker/coverage.Dockerfile -t my_image . &&
docker build -f docker/Dockerfile -t my_image .  &&
docker run --rm -p 8080:80 --name my_run my_image:latest ;
