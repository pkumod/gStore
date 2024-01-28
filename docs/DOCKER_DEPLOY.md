# Deploy gStore via Docker

There are two ways to deploy gStore via containerization:

* Pull the image which has been built automatically on the docker hub. (Recommended)

* Build the docker image via Dockerfile in the root directory of the project.

But before that, you need to have Docker on your system.

## Install Docker

### Linux

> Please refer to the Docker documentation for detailed and up-to-date install instructions of installing docker on other Linux distributions (e.g. Fedora, CentOS, Arch Linux): [English](https://docs.docker.com/install/linux/docker-ce/ubuntu/), [中文](https://docs.docker-cn.com/engine/installation/linux/docker-ce/centos/#%E5%85%88%E5%86%B3%E6%9D%A1%E4%BB%B6)

To install Docker, you would follow different steps depending on the operating system you're using. When working on Debian or Ubuntu:

1. Update your existing list of packages:

```bash
sudo apt-get update
```

2. Install a few prerequisite packages which let `apt` use packages over HTTPS:

```bash
sudo apt-get install apt-transport-https ca-certificates curl software-properties-common
```

3. Add the GPG key for the official Docker repository to your system:

```bash
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
```

4. Add the Docker repository to APT sources:

```bash
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
```

5. Update the package database with the Docker packages from the newly added repo:

```bash
sudo apt-get update
```

6. Make sure you are about to install from the Docker repo instead of the default Ubuntu repo:

```bash
apt-cache policy docker-ce
```

7. Finally, install Docker:

```bash
sudo apt-get install docker-ce
```

8. Docker should now be installed, the daemon started, and the process enabled to start on boot. Check that it's running:

```bash
sudo systemctl status docker
```

> **For mainland China Users:**
> 
> If you are experiencing issues with downloading Docker images, your life will be better with a docker mirror. You can add the mirror site by creating a daemon.json file in /etc/docker/:
> ```json
> {
>   "registry-mirrors": ["https://docker.mirrors.ustc.edu.cn", "https://hub-mirror.c.163.com"]
> }
> ```
> Then restart Docker by running `sudo systemctl restart docker`.

### macOS

1. Visit the [Docker Desktop Product Page](https://www.docker.com/products/docker-desktop).

2. Click the "Get Docker" button to download the Docker.dmg file.

3. Once the file has downloaded, open it and drag the Docker.app into your Applications folder.

4. Open Docker.app to start the Docker daemon.

### Windows

1. Visit the [Docker Desktop Product Page](https://www.docker.com/products/docker-desktop).

2. Click the "Get Docker" button to download the Docker.exe file.

3. Run the installer to install Docker Desktop.

4. After installation, Docker Desktop will start automatically.

Note: Docker Desktop for Windows requires Windows 10 21H2 (Build 19044) or higher. 

## Reuse the image (Recommended)

For most users, it is easier to pull the image from Docker Hub. You can pull the image by running the following command:

```bash
docker pull pkumodlab/gstore-docker:latest
```

Run the docker image:

```bash
docker run -p 9000:80 -it pkumodlab/gstore-docker
```

## Build from scratch

If you want to test out the latest version of gStore, you can build the image from scratch. To do so, you need to clone the repository first:

```bash
# we are doing a shallow clone here to save time and space
git clone https://github.com/pkumod/gStore
```

Then, you can build and run the image by running the following command:

```bash
docker build -t gstore .
docker run -p 9000:80 -it gstore
```