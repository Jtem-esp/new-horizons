pipeline {
	agent none

	stages {
		stage('Build') {
			agent {
				label 'os:windows'
			}

			steps {
				checkout scm
				GitLFSPull([
					lfsEndpoint: 'https://github.com/PiratesAhoy/new-horizons.git/info/lfs',
					lfsAccess: 'basic'
				])
			}
		}
	}
}