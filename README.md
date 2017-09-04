# recogPlate
RecognitionPlate  by using svm and ann</br>
First , segment :select regions of possible plates;</br>
Second,choose plates:use SVM for correct plate regions;</br>
Third,extract features: cut each plate by each character's region and extract its feature of histogram;</br>
Finally,ANN predict: use ANN model in each character's region and define its output order.</br>
