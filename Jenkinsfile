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
        sh '/valve/shell.sh --arch=i386 "make -f sp/src/everything.mak"'
      }
    }
    stage('Get Artifacts') {
      steps {
        archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
      }
    }
  }
}