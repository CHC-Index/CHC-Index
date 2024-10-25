# 1 Introduction

This is a description of the code used for the experiments described in the paper entitled Cohesiveness-aware Hierarchical Compressed Index for
Community Search on Attributed Graphs. The full version of this paper is uploaded in this repository.

We evaluated our Cohesiveness-aware Hierarchical Compressed Index (CHC-Index)
by integrate to other methods published recently, e.g., ATC, VAC, SEA, rKACS
in terms of effectiveness and efficiency for k-core and k-truss models
on attributed homogeneous graphs and attributed heterogeneous graphs.

# 2 Requirements
The experiments have been run on a Linux server with a 2.1 GHZ, 64 GB memory AMD-6272 Linux server.
Most of the algorithms are implemented with C++ and compiled by g++-12 except SEA algorithm.
The jar package is provided by the author of *Scalable Community Search with Accuracy Guarantee on Attributed Graphs*.

# 3 Code Structure
## 3.1 ./Source/include/
This folder contains the main code of CHC-Index.

In **basic_define.h**, we defined some types and enums, e.g. **Cmodel** indicates for
*core* or *truss*, **StructureMethod** indicates for whether the optimization on the distance metric (opt2) is enabled, **__Metric** indicates for 
the attribute cohesiveness metric is *HYB*(numerical & textual, e.g. VAC and SEA), or *TEXT*(just textual attributes, ATC and rKACS).

In **basic_data.h**, we define the data structure for storing vertex attributes;
in addition, we also define the data structure for storing vertex local structure information for opt2.

In **chci.h**, we have implemented the CHC-Index algorithm. We first start from *Build_Init(...)* function for loading
datasets and some basic information, and then run *build_all_layer(...)* function to build the whole CHC-Index from the
bottom up. Especially, we implemented the *robustPrune++*(opt1) by adding a conditional statement to the function *Robust_Prune_Build(...)*,
once the parameter *enable_rppp* is set to nonzero and *alpha_ > 1*, the *robustPrune++*(opt1) would be activated;
By setting the parameter *sMethod_* to *opt2*, the structure information of every vertex would be load in initialization period and
the optimization on the distance metric (opt2) would be enabled.

In **decomposition.h**, we have implemented the core decomposition and truss decomposition algorithms.

In **metric_space.h**, we have implemented the metric function.

In  **read_adj.h**, we have implemented the functionality of loading the dataset.

In **utils.h**, we have implemented some useful funcitons.

## 3.2 ./Source/integrate/
This folder contains some of the algorithms which were integrated.

## 3.3 ./Source/scripts/
This folder contains some scripts required for the experiments.

## 3.4 ./Source/tests/
This folder contains some .cpp files required for the experiments.

## 3.5 ./Source/3rdPartyLib/
This folder contains two third-party libraries, namely snap and parallel-hashmap.
Both of the libraries are required for rKACS algorithm. It is recommended to compile the snap library on a Linux system, otherwise errors may occur.

# 4 DATASETS

Our experiment involves five datasets([Dataset](https://drive.google.com/drive/folders/1CQj0IF39XbkNg0ksxTK0LihG-BMz1YNt?usp=sharing)) popularly deployed by existing works. Each dataset represents a homogeneous graph or hetergeneous graph.

Each line of the graph file represents the information on one edge, in the form of *dot-blank-dot*.

Each line of the attribute file represents the attribute information of the node, and the attribute information includes text type and continuous value type, all of which are represented by floating-point numerical type.

The Facebook, Twitch, LiveJournal are 3 homogeneous datasets.

We organized Facebook dataset in a different way. In our experiment,
the 10 facebook datasets were merged in a big dataset, and
the features of each node are merged together based on their actual semantics,
which are stated in *fb_featname.txt*. 
Specially, the edge map of FB0 dataset is the edge map of 0 ego network.

Example of fb_merge.edges:

- fb_merge.edges

| Vertex_id | Vertex_id |
| :-------: | :-------: |
|    236    |    186    |

- fb_merge.feat

| Vertex_id |                       Vertex_Attribute                       |
| :-------: | :----------------------------------------------------------: |
|    10     | [0.0 0.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 …… 1.0] |

The hetergeneous graphs contain graph files, edge files, vertex files and attribute files, which are stored in the form of txt text.

Here, DBLP is taken as an example.

Each line of the vertex file represents the type of the node, in the form of *dot-blank-dot*.

Each line of the edge file represents the type of the edge, in the form of *dot-blank-dot*.

Each line of the attribute file represents the attribute information of the node, and the attribute information includes text type and continuous value type, all of which are represented by floating-point numerical type.

Each row of the graph file represents information of all adjacent points and adjacent edges of a point in the form of *source point-blank-adjacent point 1-blank-adjacent edge number 1-blank-adjacent point 2-blank-adjacent edge number 2*.

Example of DBLP

- vertex.txt

| vertex_id | vertex type |
| :-------: | :---------: |
|     0     |      0      |

- edge.txt

| edge_id | edge type |
| :-----: | :-------: |
|    0    |     0     |

- graph.txt

| Vertex_id | adjacent_node1 | adjacent_edges_number1 | adjacent_node2 | adjacent_edges_number2 |
| :-------: | :------------: | :--------------------: | :------------: | :--------------------: |
|     2     |      962       |           6            |      141       |           7            |

- attribute.txt

| Vertex_id |                      Vertex_Attribute                      |
| :-------: | :--------------------------------------------------------: |
|     6     | [1.0 0.0 0.0 0.0 0.0 0.0 0.0 …… 1.0 0.0017621145374449338] |

In the experiment, we use specific meta path for each hetergeneous dataset.
Specially, we use 2 meta paths for IMDB dataset, named IMDB_Movie and IMDB_Person.

To prepare the datasets, download all the datasets from the link above,
and put them to the root directory of the code. After this step, there should be
two folders named "datasets" and "rKACS_ds" in the root directory.


# 5 Usage

## CHC-Index Build and Retrieve
* Build by CMake

To build the project successfully, OpenMP is required. And Snap library
is required to build the project of rKACS algorithm.
Run the script "build.sh" in the "shells" directory, the project
would be compiled.

Note that if the Snap library could not be found, the build may failed, as 
this library is required by the rKACS algorithm. You could step into /3rdPartyLib/snap
and install the Snap library, otherwise you could
delete all content in the "CMakeList.txt" after line 73 to resolve this issue.


* Build CHC-Index for specific dataset and algorithm

Most of the specific parameters could be assigned in "datasaet_args.sh".


./Build < dataset_name > <index_path > < graph_path > < vertex_path >
< edge_path > < feat_path > < degree > < L > < alpha > < beta > < enable_rp++ >
< thread_num > < community_model > < apply_opt2 > < metric >

1. dataset_name: The name of the dataset, e.g. FB0, Facebook, DBLP, Twitch, IMDB_Person, IMDB_Movie, LiveJournal,
2. index_path: The path to store the built index.
3. graph_path, vertex_path, edge_path, feat_path: The information of dataset.
4. degree: The degree for each layer of CHC-Index.
5. L: The search queue length while constructing the index.
6. alpha: Parameter of vamana indexing algorithm.
7. beta: Parameter of robustPrune++.
8. enable_rp++: set to 0 if you want to use the origin vamana indexing algorithm.
9. thread_num: The num of threads while constructing the index.
10. community_model: core or truss
11. apply_opt2: set to "A" if you want to enable the second optimization in the paper, otherwise set to "baseline"
12. metric: set to "TEXT" while the algorithm is rKACS or ATC; set to "HYB" while the algorithm is SEA or VAC.

* Retrieving nodes via the CHC-Index

After the CHC-Index is successfully built, you can retrieve nodes from it.
./Retrieve < dataset_name > <index_path > < graph_path > < vertex_path >
< edge_path > < feat_path > < retrieve_result_path > < query_id > < r > < layer >
< community_model > <apply_opt2> < metric >

1. dataset_name: The name of the dataset, e.g. Facebook, DBLP, ...
2. index_path: The path of one built index.
3. graph_path, vertex_path, edge_path, feat_path: The information of dataset.
4. retrieve_result_path: Path to store the retrieved nodes.
5. query_id: The id of the query in the dataset.
6. r: The parameter.
7. layer: The num of layer that you want to retrieve nodes from.
8. community_model: core or truss
9. apply_opt2: set to "A" if you want to enable the second optimization in the paper, otherwise set to "baseline"
10. metric: set to "TEXT" while the algorithm is rKACS or ATC; set to "HYB" while the algorithm is SEA or VAC.

## Easier way to run

We provide a easier way to run the algorithms by a shell file called
"run_algo.sh".

./run_algo.sh < dataset_name > < algorithm_name > < operation >

1. dataset_name: the dataset_name could be found in "config_dataset.sh", like FB0, Facebook, DBLP, Twitch, IMDB_Person, IMDB_Movie, LiveJournal.
2. algorithm_name: the community search algorithm that you want
   to integrate, including VAC, SEA, ATC, rKACS.
3. operation: the operation you want to do, including
* build: build the whole CHC-Index.
* retrieve: retrieve some nodes from the built index.
* VAC, SEA, ATC, rAKCS: Run the corresponding algorithm
  with the retrieved nodes in "retrieve" step. Specially,
  you could choose whether to run the origin algorithm without
  CHC-Index by set the "run_wo" parameter to 0 or 1 in the "run_algo.sh" file.


e.g.: To run VAC algorithm on DBLP with CHC-Index, first build
the index by executing **./run_algo.sh DBLP VAC build**, then retrieve
nodes for queries by executing **./run_algo.sh DBLP VAC retrieve**, finally
run VAC algorithm with the retrieved nodes by executing
**./run_algo.sh DBLP VAC VAC**. (If you want, you could run
**./run_algo.sh DBLP VAC ATC** to run *ATC* algorithm with the nodes
retrieved in *VAC* metric, but it may cause a bad result :( ).

## The query result

1. Build: The built index would be output to /exp_data/indices/<dataset_name>
2. Retrieve: The retrieve result would be output into /exp_data/retrieve/<dataset_name>. For each query,
   the result contains 2 lines, the first line is query_id, and the following line
   is composed of the retrieved nodes.
3. Community algorithms: The result of community search would be stored in /exp_data/<algo_name>/<dataset_name>.
   For each query, the result contains 4 lines: the first line is query_id, the second line is composed
   of the nodes in the community, the third line is the score of the community, and the
   fourth line is the runtime. Specially, for rKACS algorithm, the id of nodes in the
   result is reordered, which would not affect the experiment.

P.S.: The rKACS program requires for snap library, the snap library
should be built firstly in 3rdPartyLib/snap, otherwise the rKACS algorithm could not
run successfully.

# 5 Experimrent
## Section 5.2 & 5.3

First, run
./run_algo.sh DBLP VAC build
to build the index.

The experiment could be reproduced by adjusting the parameter r
and setting *LAYER* to 15.

Then run (take DBLP, VAC as an example):
./run_algo.sh DBLP VAC retrieve
./run_algo.sh DBLP VAC VAC

## Section 5.4

By setting the parameter *RPPP* to *0* and *apply_opt2* to *baseline*
in the shell script in the index constructing step, we get the
Index w/o O1+O2 version; By setting the parameter *RPPP* to *1* and *apply_opt2* to *baseline*
in the shell script in the index constructing step, we get the
Index w/o O2 version;By setting the parameter *enable_rp++* to *1* and *apply_opt2* to *A*
in the shell script in the index constructing step, we get the
Index with all the optimizations;


Then run (take DBLP, VAC as an example):
./run_algo.sh DBLP VAC build
./run_algo.sh DBLP VAC retrieve
./run_algo.sh DBLP VAC VAC

## Section 5.5

The index construction and storage overhead could be seen in
the former index build steps.

## Section 5.6

The case study was running on VAC algorithm. By setting the *dataset* to
*FB0*, *query_id* to *20*, *LAYER* to *4*, the experiment could be reproduced.

Run:
./run_algo.sh FB0 VAC build
./run_algo.sh FB0 VAC retrieve
./run_algo.sh FB0 VAC VAC

## Section 5.7

The experiment could be reproduced by varying the parameter to the given value, allowing for consistent results.
The parameter *r*, *LAYER*, *BETA* could be changed.

Similarly, take DBLP, VAC as an example, after each change of a parameter, run:
./run_algo.sh DBLP VAC retrieve
./run_algo.sh DBLP VAC VAC

# Future Work
More of the code used in the experiment will be released in a few days.
