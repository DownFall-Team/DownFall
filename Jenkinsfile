def GitURL = "https://github.com/DownFall-Team/DownFall.git"
def GitCredentialsId = 'b5f4f865-2a92-418e-8467-7a271a884349'
def SrcPath = "sp/src"
def GameBinFolder = "sp/game/downfall/bin"
def VPCScript = "creategameprojects"
def SolutionName = "games"

stage ('Build Downfall') {
	parallel "Linux": {
		node ('linux') {
			stage('Git') {
				git url: GitURL, credentialsId: GitCredentialsId
			}

			stage('Generate VPC on Linux') {
				dir (SrcPath) {
					sh "./${VPCScript}"
				}
			}

			stage('Compile on Linux') {
				sh "if [ -d ${GameBinFolder} ]; then echo \"Dir already exists...\"; else mkdir -p ${GameBinFolder}; fi"

				dir ('sp/src') {
					sh "/valve/steam-runtime/shell.sh --arch=i386 make -f ${SolutionName}.mak"
				}
			}

			stage('Get Artifacts on Linux') {
				archiveArtifacts(artifacts: GameBinFolder, onlyIfSuccessful: true)
			}
		}
	},
	"Windows": {
		node ('windows') {
			stage('Git') {
				git url: GitURL, credentialsId: GitCredentialsId
			}

			stage('Generate VPC on Windows') {
					dir (SrcPath) {
						bat "${VPCScript}.bat"
						bat "copy ${SolutionName}.sln+sln_fix.txt ${SolutionName}.sln"
				}
			}

			stage('Compile on Windows') {
				bat "if not exist ${GameBinFolder} (mkdir ${GameBinFolder})"
				
				dir ('sp/src') {
					bat """call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Enterprise\\Common7\\Tools\\VsDevCmd.bat"
					msbuild ${SolutionName}.sln /t:Build /p:Configuration=Release /m:4"""
				}
			}

			stage('Get Artifacts on Windows')
			{
				archiveArtifacts(artifacts: "${GameBinFolder}/*.dll", onlyIfSuccessful: true)
			}
		}
	}
}