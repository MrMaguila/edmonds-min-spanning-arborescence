# edmonds-min-spanning-arborescence

A simple (and unoptimized) implementation of the Edmonds algorithm for finding a minimum spanning arborescence (https://en.wikipedia.org/wiki/Edmonds%27_algorithm#:~:text=In%20graph%20theory%2C%20Edmonds%27%20algorithm,the%20minimum%20spanning%20tree%20problem.).

Having g++ installed, you just need to execute the following line in a terminal to run it:

`bash build_and_run.sh`

There's also a python script (`./graphs/generate_graphs.py`) for generating random graphs, to execute it
you need to install poetry (https://python-poetry.org) first and run the following command in the `./graphs` directory:

`poetry install --no-root`

It will create a virtual env with the required dependencies. Then you can run the script with:

`poetry run python generate_graphs.py`
