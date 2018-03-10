pipeline {
  agent any
  stages {
    stage('Dependencies') {
      parallel {
        stage('Apt-Gets') {
          steps {
            sh 'apt-get install build-essential gcc-multilib g++-multilib'
          }
        }
        stage('Steam Runtime') {
          steps {
            sh '''cd /
mkdir valve
cd valve
wget http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz
tar xvf steam-runtime-sdk_latest.tar.xz
mv steam-runtime-sdk_2013-09-05 steam-runtime
chown jenkins:jenkins * -R
./setup.sh'''
          }
        }
      }
    }
  }
}