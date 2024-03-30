pipeline {
	agent none

	stages {
		stage('Build') {
			agent {
				label 'os:windows'
			}

			steps {
				checkout scm
				dir("installer") {
					bat "python publish.py"
				}
			}
		}
	}
}