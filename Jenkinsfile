stage ('Build Downfall') {
	parallel "Linux": {
		node ('linux') {
			stage('Git') {
				git url: 'https://github.com/DownFall-Team/DownFall.git'
			}

			stage('Generate VPC on Linux') {
				dir ('sp/src') {
					sh './creategameprojects'
				}
			}

			stage('Compile on Linux') {
				sh 'mkdir -p sp\\game\\bin'
				dir ('sp/src') {
					sh '/valve/steam-runtime/shell.sh --arch=i386 make -f games.mak'
				}
			}

			stage('Get Artifacts on Linux') {
				archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
			}
		}
	},
	"Windows": {
		node ('windows') {
			stage('Git') {
				git url: 'https://github.com/DownFall-Team/DownFall.git'
			}

			stage('Generate VPC on Windows') {
					dir ('sp/src') {
						bat 'creategameprojects.bat'
						bat 'copy game.sln+sln_fix.txt game.sln'
				}
			}

			stage('Compile on Windows') {
				bat 'mkdir -p sp\\game\\bin'
				dir ('sp/src') {
					bat '''call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
					msbuild game.sln /t:Build /p:Configuration=Release /m:4'''
				}
			}

			stage('Get Artifacts on Windows')
			{
				archiveArtifacts(artifacts: 'sp/game/downfall/bin', onlyIfSuccessful: true)
			}
		}
	}
}