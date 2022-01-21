pipeline {
  agent any
  stages {
    stage('stage1') {
      parallel {
        stage('stage1') {
          steps {
            sh 'echo "Step 1"'
          }
        }

        stage('stage 1.1') {
          steps {
            echo 'Step 1.1'
          }
        }

      }
    }

    stage('stage2') {
      steps {
        sleep 5
      }
    }

    stage('stage3') {
      steps {
        echo 'Hello World'
      }
    }

  }
}