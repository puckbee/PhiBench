USAGE:
	You need to train the classifier first,then,use the classifier to classify dataset;
TODO:
	1. The robustness of this program is not so good , need to add some code
	to process the unexpected input data;
	2. Hostd and single thread running ,time consuming : 
		train: 3.8s
		classify: 13m41s
	3. Mic-native and multi-thread running , time consuming: 
		train: 34s
		classify: 5m19s
	4. Offload and multi-thread running, time consuming:
		train: 6.3s
		classify: 1m17s
	Mic-native and multi-thread running,time-consuming is mostly costed on
	reading data and writing data.
	5. The openmp code block can exchange the order of the for-loop, and fold
	them that could be folded to one for-loop.
NOTICE:
	1. The training process is not fitable for parallel,because of the for
	loop is too short to deserialize. Experiment shows that for loop if deserialized,performance will turned down;

BUG-FIX:
	1. Fix the classify problem,Mainly caused by the Data-Competition on the
	array array_test_class_probability.
