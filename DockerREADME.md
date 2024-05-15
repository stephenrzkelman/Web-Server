## Background
This is the docker container used for deployment.


# How to run Docker
This must be performed on your local machine. This is not to be done within the usual CS130 development environment.

## Step 0: Enter project
    cd fortnite-gamers
Before anything, make sure you are in the repo. Make sure your local machine has Docker running.

## Step 1: Build base image

    docker build -f docker/base.Dockerfile -t fortnite-gamers:base .
Build the original base image named fortnite-gamers, tagged with base.

## Step 2: Build builder/deploy image
    docker build -f docker/Dockerfile -t my_image .  
Here, you use the project Dockerfile and make the builder/deploy image. my_image is the tag, but it doesn't matter what you name this, my_image is just a placeholder.

## Step 3: Run image
    docker run --rm -p 8080:80 --name my_run my_image:latest    
This command will run the docker image created and now you can get to testing.
Rm is important as it will delete the container afterwards. ***my_run*** is the name of the containers run. You may change it but make sure it is ***consistent***.

## Step 4: Test behavior
    curl localhost:8080   
***This test above is specifically for the skeleton code and basic docker image***
Open a new terminal and type this command in. With the Docker image running this should be a simple HTTP GET echo listener.

## Step 5: Stop the container
    docker container stop my_run
Remember to stop the container, if steps were followed properly earlier then the container will automatically be deleted. Hooray.
