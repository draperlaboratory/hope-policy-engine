# Validator

The validator directory contains the framework for building validators, as well as concrete validators.

The validator framework at its highest level provides an extremely simple validate/commit interface.
Derivations of this root validator provide tag based validation semantics.  The architecture is
designed to allow for many implementations, on multiple ISAs.  Provided in this release is a Renode
loadable validator that can deal with RISCV32 platforms.

The concrete build product from this directory, currently, is a shared object that is loadable by
Renode to provide validation for RV32 platforms.  There is also a standalone application that is
independent of Renode that allows for simple testing of policy code.

## Standalone App

The standalone application (in `riscv/standalone.cc`) is designed to be run completely independent
of Renode.  No application is loaded, and no actual simulation of a RISCV processor is done.  The
application iterates over a stream of hand coded instructions, with manually generated tags,
pretending to execute each, by simply advancing a PC, and updating a fake register state according
to manual coding via a list of operations in a data structure.

The application should be useful for debugging simple test cases for policies without needing to involve
the entirety of Renode.
