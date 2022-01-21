pipeline {
  agent {
    node {
      label 'Node2'
    }

  }
  stages {
    stage('stage1') {
      parallel {
        stage('stage1') {
          steps {
            sh 'echo "Step 1"'
            node(label: 'Not1.1') {
              echo 'Add node'
              sleep 3
            }

          }
        }

        stage('stage 1.1') {
          steps {
            echo 'Step 1.1'
            waitUntil(initialRecurrencePeriod: 5, quiet: true) {
              echo 'qqq'
            }

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
  environment {
    java = '11'
  }
}