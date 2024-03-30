WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
# -*- coding: utf-8 -*-

import pandas as pd
import numpy as np
import standard_classification_sample as scs
from sklearn.linear_model import  LogisticRegression
from sklearn.svm import SVC

def csv_data(filename):
    df = pd.read_csv(filename)
    #data = df.as_matrix(columns = ['Ambiant Temp C', 'Object Temp C'])
    #data = df.as_matrix(columns = [ 'Object Temp C'])
    data = df.as_matrix(columns = [ 'Brightness']) 
    return data



################################################################################
if __name__ == "__main__":

    # Make binary data from two reading files

	#Obtain the brightness samples obtained at the bright location
    X_bright = csv_data('record-bright.csv') 
    Y_bright = np.ones(X_bright.shape[0])

	#Obtain the brightness samples obtained at the dark location
    X_dark = csv_data('record-dark.csv')
    Y_dark = np.zeros(X_dark.shape[0])

    X = np.concatenate((X_bright, X_dark), axis=0)
    Y = np.concatenate((Y_bright, Y_dark), axis=0)
	
	# Now complete this function! 
	# Train and tuning different classifiers, and print their accuracy.

    # Split to training and testing groups
    data1, data2, target1, target2 = scs.train_test_split(X, \
                                                      Y, \
                                                      test_size=0.8, \
                                                      random_state=42)
    
    print(data1)
    # Train classifiers
    verbosity = 3
    
    clf_svm_lin = scs.tune_svm_lin(data1, target1, verbose=verbosity)

    # You should complete the tune functions for the other three classifiers
    # For instance, 
    clf_knn = scs.tune_knn(data1, target1, verbose=verbosity)
    clf_decision_tree = scs.tune_decision_tree(data1, target1, verbose=verbosity)
    clf_random_forest = scs.tune_random_forest(data1, target1, verbose=verbosity)
    
    
    # Print performance
    print ""
    print "Classifier Performance"
    print "--------------------------------------------------------------------------------"
    print ""
   
    print ""
    print "Accuracy with linear SVM and parameter tuning:"
    scs.print_accuracy(clf_svm_lin, data1, target1, data2, target2)

    print ""
    print "Accuracy with k-nn classifier with  parameter tuning:"
    # You should print the accuracy of your knn classifer here
    scs.print_accuracy(clf_knn, data1, target1, data2, target2)
  

    print ""
    print "Accuracy with a decision tree and parameter tuning:"
    # You should print the accuracy of your decision tree classifer here
    scs.print_accuracy(clf_decision_tree, data1, target1, data2, target2)

    
    print ""
    print "Accuracy with a random forest and parameter tuning:"
    # You should print the accuracy of your random forest classifer here
    scs.print_accuracy(clf_random_forest, data1, target1, data2, target2)

   

    
