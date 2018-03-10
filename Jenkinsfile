pipeline {
  agent any
  stages {
    stage('Dependencies') {
      parallel {
        stage('Apt-Gets') {
          steps {
            sh 'sudo apt-get install build-essential gcc-multilib g++-multilib'
          }
        }
        stage('Steam Runtime') {
          steps {
            sh '''cd /
sudo mkdir valve
cd valve
sudo wget http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz
sudo tar xvf steam-runtime-sdk_latest.tar.xz
sudo mv steam-runtime-sdk_2013-09-05 steam-runtime
sudo chown jenkins:jenkins * -R
./setup.sh'''
          }
        }
      }
    }
  }
}