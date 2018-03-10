pipeline {
  agent none
  stages {

    stage('Generate VPC') {
      parallel {

        stage('Generate VPC on Windows') {
          agent { label "windows" }
          steps {
            dir ('sp/src') {
              bat 'createallprojects.bat'
            }
          }
        }

        stage('Generate VPC on Linux') {
          agent { label "linux" }
          steps {
            dir ('sp/src') {
              sh './createallprojects'
            }
          }
        }

      }
    }

    stage('Build All Projects') {
      parallel {

        stage('Build All on Windows') {
          agent { label "windows" }
          steps {
            dir ('sp/src') {
              bat '''"
              call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
              msbuild everything.sln /t:Build /p:Configuration=Release /m:6'''
            }
          }
        }

        stage('Build All on Linux') {
          agent { label "linux" }
          steps {
            dir ('sp/src') {
              sh '/valve/steam-runtime/shell.sh --arch=i386 make -f everything.mak'
            }
          }
        }

      }
    }

    stage('Get Artifacts')
    {
      parallel {

        stage('Get Windows Artifacts')
        {
          agent { label "windows" }
          steps {
            archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
          }
        }

        stage('Get Linux Artifacts')
        {
          agent { label "linux" }
          steps {
            archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
          }
        }

      }
    }

  }
}