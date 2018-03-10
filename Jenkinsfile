stages {
	stage('Build Source') {

		parallel {

			stage('Build for Windows') {

				node('windows')
				{
					stage('Generate VPC on Windows') {
						steps {
							dir ('sp/src') {
								bat 'createallprojects.bat'
							}
						}
					}

					stage('Compile on Windows') {
						steps {
							dir ('sp/src') {
								bat '''
								call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
								msbuild everything.sln /t:Build /p:Configuration=Release /m:6'''
							}
						}
					}

					stage('Get Artifacts on Windows')
					{
						steps {
							archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
						}
					}
				}

			}

			stage('Build for Linux') {

				node('linux')
				{
					stage('Generate VPC on Linux') {
						steps {
							dir ('sp/src') {
								sh './createallprojects'
							}
						}
					}

					stage('Compile on Linux') {
						steps {
							dir ('sp/src') {
								sh '/valve/steam-runtime/shell.sh --arch=i386 make -f everything.mak'
							}
						}
					}

					stage('Get Artifacts on Linux') {
						steps {
							archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
						}
					}
				}

			}

		}

	}
}