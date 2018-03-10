pipeline {
  agent none
  stages {
    stage('Windows') {
      agent { node 'windows' }

      stage('Generate VPC') {
        steps {
          dir 'sp/src'
          bat 'createallprojects.bat'
        }
      }
      stage('Build All Projects') {
        steps {
          dir 'sp/src'
          bat '"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\IDE\\devenv.exe" everything.sln /build Release'
        }
      }
      stage('Get Artifacts') {
        steps {
          archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
        }
      }

    }
    stage('Linux') {
      agent { node 'linux' }
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
}