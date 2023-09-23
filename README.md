# libfunctional_dag (or lambdag $\lambda$g)

A functional dag is a multi-threaded design pattern that runs simple $\lambda$ functions, enforced to be structured as a directed acyclic graph (DAG). This project enforces one other important quality which is being strongly typed. Users of this library will know, right away, that types are incompatible during compile time which reduces runtime checking during message passing between the nodes.

## Why would you want this?
Cognitive architectures can become very complicated very fast. Typically when designing these complex cognitive architectures, we're left with either a single path through the code netowrk (e.g. a neural network) or giant speghetti code architectures used in research that are more experimental. This library was built primarily with cognitive perception systems in mind. While not impossible, it is challenging to push information back to early pre-processing stages. This constraint makes users of this library think twice about what assumptions they've made in which state needs to be transmitted back up the DAG. On the other hand, it is also more expressive than typical AI pipelines that force data through the entire network regardless of whether the data should be transmitted. This flow pattern is ideal for some cognitive systems like robotic vision and some single direction modules within larger cognitive architectures.

### Build dependencies
This project tries to minimize dependencies so as to not stack dependencies across larger projects and to make it easier to build simple layers to other languages like python. 

Build dependencies:
* Cmake
* jsoncpp 
* ctest (if you want to unit test)
* clang-tidy (if you want to lint the code as well)

Dependencies during testing:
* Catch2 (for unit testing)

Runtime dependencies
* jsoncpp

### How to build the code
The build is CMake based and is built for cxx20 standards and we currently only support [homebrew](https://brew.sh/). 

``` bash
$ git clone https://github.com/petrogly-ph/functional-dag
$ cd functional-dag
$ brew install --build-from-source Formula/functional-dag.rb
```

### How to contribute to the code
We work on issues. If you'd like to help out, take a look at the [issues](https://github.com/petrogly-ph/functional-dag/issues), assign a free one and start coding! 

You're also welcome to perform unsolicited push requests to main and ask for a review. We'll be happy to take a look and provide feedback.

You can also help out by simply providing bug reports! 