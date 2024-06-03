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
