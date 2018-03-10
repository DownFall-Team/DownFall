pipeline {
  agent any
  stages {
    stage('Generate VPC') {
      steps {
        sh 'sp/src/createallprojects'
      }
    }
    stage('Build All Projects') {
      steps {
        sh '''cd sp/src
/valve/steam-runtime/shell.sh --arch=i386 make -f everything.mak'''
      }
    }
    stage('Get Artifacts') {
      steps {
        archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
      }
    }
  }
}