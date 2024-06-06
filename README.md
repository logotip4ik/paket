# Paket

Encrypt and decrypt folder and files. Only one file as output. It will eat everything, your computer
and even you...

## Repo

It should be a monorepo, currently only wip core is present.

- [`core`](./core) (wip) C++ library to encrypt and decrypt, serialize and deserialize files and
    folders. This should be used by future frontend (native app) build with nim ?

- [`frontend`](./frontend) (wip) C++ native gui app to interact with Paket core lib. This allows
    user to encrypt and decrypt files and folders via easy interface. Enter a master key, drag and
    drop folder or file and that's it, your items will be encrypted and stored in separate file
    with extension `.pkt`

<sub>This is my course project, so don't judge too strictly<sub>


## How to...

### Run

1. Firstly you need to build the core in `lib` mode. This can be done with:

    ```bash
    cd ./core
    make lib
    ```

    <sub>you can only build core in `release` mode by prefixing the `make lib` with `make release-lib`</sub>

2. Then, you need to build the frontend part with:

    ```bash
    cd ./frontend
    make
    ```

    <sub>This command also has `release` mode (`make release`), which will use `release` lib output of core</sub>

3. Now you can launch the gui with `./debug/paket` or `./release/paket`. (i haven't yet checked out
   how to compile the frontend so that you can just click on it and open the app...)


## Todo:

- [ ] app icon
- [ ] multithreading
- [ ] put `isFolder` flag into `attrs`
