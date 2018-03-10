stage ('Build Downfall') {
	parallel "Linux": {
		node ('linux') {
			stage('Generate VPC on Linux') {
				dir ('sp/src') {
					sh './createallprojects'
				}
			}

			stage('Compile on Linux') {
				dir ('sp/src') {
					sh '/valve/steam-runtime/shell.sh --arch=i386 make -f everything.mak'
				}
			}

			stage('Get Artifacts on Linux') {
				archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
			}
		}
	},
	"Windows": {
		node ('windows') {
			stage('Generate VPC on Windows') {
					dir ('sp/src') {
						bat 'createallprojects.bat'
						bat 'copy everything.sln+sln_fix_everything.txt everything.sln'
				}
			}

			stage('Compile on Windows') {
				dir ('sp/src') {
					bat '''call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
					msbuild everything.sln /t:Build /p:Configuration=Release /m:4'''
				}
			}

			stage('Get Artifacts on Windows')
			{
				archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
			}
		}
	}
}