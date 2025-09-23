# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++ header-only library implementing cryptographic trapdoor sets and Boolean algebras. The library focuses on oblivious data structures that provide privacy-preserving set operations through one-way hash transformations.

## Core Concepts

The library implements:
- **Trapdoor types**: One-way transformations that map values to trapdoor representations using cryptographic hashing
- **Trapdoor Boolean algebras**: Privacy-preserving set operations (union, intersection, complement) on trapdoor-encoded sets
- **Oblivious sets**: Data structures supporting approximate membership queries with controlled false positive/negative rates

## Architecture

Key components in `include/cipher_trapdoor_sets/`:
- `trapdoor.hpp`: Core trapdoor type with hash-based one-way transformation
- `trapdoor_boolean_algebra.hpp`: Boolean algebra operations on trapdoor sets
- `trapdoor_symmetric_difference_group.hpp`: Group operations using XOR
- `trapdoor_seq.hpp`: Sequence operations on trapdoors
- `trapdoor_tag.hpp`: Tagging system for trapdoors
- `random_trapdoor.hpp`: Random trapdoor generation

## Development Commands

This is a header-only C++ library without build configuration files. To use:
- Include headers directly: `#include <cipher_trapdoor_sets/trapdoor.hpp>`
- Requires C++17 or later
- No external dependencies beyond standard library

## LaTeX Documentation

The `paper/` directory contains LaTeX documentation:
- Main file: `paper/main.tex`
- Sections in `paper/sections/`: Mathematical foundations and proofs
- Research materials in `research/`: Mathematica notebooks and figures

## Key Design Principles

- **One-way transformations**: Trapdoor functions are computationally easy to compute but hard to invert
- **Approximate equality**: Operations use probabilistic models with quantifiable error rates
- **Homomorphic properties**: Boolean operations preserve structure under encryption
- **Type safety**: Heavy use of C++ templates for compile-time guarantees