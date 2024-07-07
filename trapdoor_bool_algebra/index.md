---
title: "A Boolean Algebra Over Trapdoors"
abstract: "This paper introduces a Boolean algebra framework over trapdoors, establishing a novel approach to cryptographic operations within a Boolean algebraic structure. The core of the framework is the Boolean algebra \\(A := (\\mathcal{P}(X^*), \\land, \\lor, \\neg, \\emptyset, X^*)\\), with \\(\\mathcal{P}\\) representing the powerset and \\(X^*\\) the free semigroup on an alphabet \\(X\\). A key feature of this study is the homomorphism \\(F : A \\mapsto B\\) from \\(A\\) to a Boolean algebra \\(B := (\\{0,1\\}^n, \\&, |, ~, 0^n, 1^n)\\) of \\(n\\)-bit strings, achieved through a cryptographic hash function. This homomorphism introduces a secret \\(s\\) into its operation, embedding security within the algebraic structure and rendering \\(F\\) one-way. Our exploration highlights the cryptographic utility of this framework, especially in terms of collision probability and resistance to reverse engineering, offering a foundational basis for secure cryptographic operations leveraging Boolean algebra."
author: admin
date: '2023-06-17'
featured: no
comments: yes
draft: no
slug: trapdoor-boolean-model
image:
  caption: ''
  focal_point: ''
  preview_only: yes
categories:
- Rate-distortion
- Compression
- bernoulli
- Bernoulli Model
- Probabilistic Data Structure
- Cryptography
projects:
- /project/bernoulli-model/
header-includes:
  - "\\usepackage{amsmath}"
  - "\\usepackage{amssymb}"
  - "\\usepackage{amsthm}"
projects: []
---

This project is available on [GitHub](https://github.com/queelius/bernoulli_data_type/boolean-algebra-trapdor/).

## Introduction


## Boolean Algebra

A Boolean algebra is a mathematical structure that captures the properties of logical operations and sets. Formally, it is defined as a 6-tuple $(B, \land, \lor, \neg, 0, 1)$, where

- $B$ is a set of elements,
- $\land$ ($\rm{and}$) and $\lor$ $\rm{or}$ are binary operations on $B$,
- $\neg$ ($\rm{not}$) is a unary operation on $B$,
- $0$ and $1$ are elements of $B$, often referred to as the minimum and maximum elements, respectively.

These components must satisfy certain axioms, including closure of $B$ under the operations, commutativity, associativity, distributivity, and the existence of identity and complement elements [1].

Boolean algebras have far-reaching application. They form the foundation of propositional logic and are fundamental to the design of digital circuits and computer architecture [2].

In set theory, a common representation of a Boolean algebra is the power set of a set $X$, denoted $\mathcal{P}(X)$, with the following correspondence:

- $B = \mathcal{P}(X)$,
- $\land = \cap$ (set intersection),
- $\lor = \cup$ (set union),
- $\neg = \complement$ (set complement),
- $0 = \emptyset$ (empty set),
- $1 = X$ (universal set).

This set-theoretic Boolean algebra, $(\mathcal{P}(X), \cap, \cup, \complement, \emptyset, X)$, is a canonical example of a Boolean algebra. It will be the starting point for our exploration of an Boolean algebra over trapdoors [3]. This structure will maintain the familiar properties of Boolean algebras while introducing cryptographic elements for secure computations and data structures.

## Homomorphisms in Boolean Algebra

A key concept in our exploration of trapdoor Boolean algebras is that of a homomorphism. In abstract algebra, a homomorphism is a structure-preserving map between two algebraic structures of the same type. In the context of Boolean algebras, a homomorphism is a function that preserves the operations and special elements of the Boolean algebra.

Given two Boolean algebras $(A, \land_A, \lor_A, \neg_A, 0_A, 1_A)$ and $(B, \land_B, \lor_B, \neg_B, 0_B, 1_B)$, a function $f: A \to B$ is a Boolean algebra homomorphism if for all $x, y \in A$:

1. $f(x \land_A y) = f(x) \land_B f(y)$
2. $f(x \lor_A y) = f(x) \lor_B f(y)$
3. $f(\neg_A x) = \neg_B f(x)$
4. $f(0_A) = 0_B$
5. $f(1_A) = 1_B$


In other words, a homomorphism preserves the structure of the Boolean algebra across the mapping. This preservation of structure is crucial as it allows us to perform operations in one Boolean algebra and have them correspond meaningfully to operations in another Boolean algebra [4].

Homomorphisms play a vital role in our construction of trapdoor Boolean algebras. They allow us to create a mapping between our original Boolean algebra and a new structure that incorporates cryptographic elements, while still maintaining the essential properties of a Boolean algebra. This preservation of structure ensures that operations performed in our trapdoor Boolean algebra still behave in ways that are logically consistent with standard Boolean operations.

In the following sections, we will introduce a specific homomorphism $F$ that maps elements from our original Boolean algebra to a Boolean algebra over bit strings, incorporating a cryptographic hash function. This homomorphism will be the foundation of our Boolean algebra over trapdoors, allowing us to perform Boolean operations in a way that leverages cryptographic properties.

## The Bernoulli Model

Before we introduce our Bernoulli homomorphism, it's crucial to understand the underlying framework: the Bernoulli model.

The Bernoulli model is a probabilistic framework for representing and reasoning about approximations of values (where a value can be anything from a set, whether the set is something simple like $\\{\rm{true}, \rm{false}\\}$ or the set of functions of type $X \to Y$ (we call these Bernoulli maps in the general case).

It introduces a kind of controlled uncertainty into computations, allowing for trade-offs between accuracy and other desirable properties such as space efficiency or security.

Formally, for any type $\mathcal{T}$, we define its Bernoulli approximation as
$$
B_{\mathcal{T}}.
$$
When we observe a value from the Bernoulli Model process, we may denote the value as $B_{\mathcal{T}}(x)$, where the value $x$ is the latent value of type $\mathcal{T}$ that we cannot observe directly. The Bernoulli model introduces an error probability $\epsilon(x)$, which represents the probability that the approximation $B_{\mathcal{T}}(x)$ is erroneous:
$$
\Pr\bigl\\{B_{\mathcal{T}}(x) \neq x \bigr\\} = \epsilon(x).
$$

We may want to discuss the number of ways in which the Bernoulli Model can introduce errors. For a first-order Bernoulli model, $\epsilon(x) = \epsilon$ for all $x \in \mathcal{T}$, i.e., it is a constant (and usually known) probability. In this case, we say that the *order* of the Bernoulli model is $k=1$. For instance, if we had a noisy binary symmetric channel, we might have $\epsilon = 0.1$ for a given channel, and so whenever we observe a value from the channel, we know that there is a 10% chance the value is erroneous. For higher-order models, $\epsilon : \mathcal{T} \mapsto [0,1]$ represents error probabilities under different conditions.

Key properties of the Bernoulli Model include:

1. Propagation of uncertainty: When operations are performed on Bernoulli approximations, the uncertainties combine in well-defined ways.

2. Trade-off between accuracy and other properties: By adjusting the probabilities, we can balance accuracy against other desirable characteristics of our system.

3. Generalization to complex types: The Bernoulli model can be applied to simple types like Boolean values, as well as to more complex types including functions and algebraic structures.

In the context of our work on an approximate Boolean algebra over trapdoors, the Bernoulli Model provides a framework for analyzing and reasoning about the behavior of our cryptographic constructions.

## A Boolean Algebra Over Free Semigroups

Consider the Boolean algebra
$$
    A := (\mathcal{P}\bigl(X^{\*}), \land = \cap, \lor = \cup, \neg = \complement, 0 = \emptyset, 1 = X^{\*}\bigr)
$$
where $\mathcal{P}$ is the powerset, $X$ is an alphabet, and $X^{\*}$ is the  free semigroup on $X$ which is closed under concatenation,
$$
    \\# : X^{\*} \times X^{\*} \mapsto X^{\*}.
$$

For example, if
$$
    X = \\{a,b\\}
$$
then
$$
    X^{\*} = \\{\epsilon, a, b, aa, ab, ba, bb, aaa, aab, \ldots \\}
$$
and
$$
    \mathcal{P}(X^\*) = \bigl\\{\emptyset, \\{\epsilon\\}, \\{a\\}, \\{b\\},
             \\{aa\\}, \ldots, \\{\epsilon,a\\}, \ldots,
             \\{a,a,babb\\}, \ldots \bigr\\},
$$
where $\epsilon$ is the empty string.

Commonly, for Boolean alebras over finite sets, one approach is to represent sets with bit strings of length $n$ and map each set to a unique bit string, e.g., the mapping $\mathbb{A} \mapsto a_1 \ldots a_n$ where bit $a_j = 1$ if the $j$-th element (according to some ordering) is in $\mathbb{A}$ and otherwise $a_j = 0$. If we have $n$ bits, we can uniquely represent up to $2^n$ sets.

In our case, we have a Boolean albebra over the infinitely large free semigroup $X^{\*}$. Our intention is to stick with the finite bit string representation, and say that the representation contains a type of controllable error on membership queries known as the the false positive rate (which is the probability that an element not in the set falsely tests positive for membership), assuming that elements are selected randomly from the free semigroup $X^{\*}$.

Conceptually, we have an approximate Boolean algebra over the free semigroup $X^{\*}$. We map element in $X^{\*}$ to a bit string in $\\{0,1\\}^2$ using a cryptographic hash function $h : X^{\*} \mapsto \\{0,1\\}^2$ such that if we randomly choose two elements in $X^{\*}$, the probability that they map (hash) to the same value (collide) is $2^{-n}$. Collisions are the fundamental source of error in our model and what also qualifies the approximate Boolean algebra as a [Bernoulli Model](https://github.com/queelius/bernoulli_data_type) process.

## Approximate Boolean Algebra Over Trapdoors

Consider the Boolean alegebra
$$
    B := \bigl(\\{0,1\\}^n, \\&, |, \sim, 0^n, 1^n\bigr)
$$
where $\\&$, $|$, and $\sim$ are defined on $\\{0,1\\}^n$ as the bitwise $\operatorname{and}$, $\operatorname{or}$, and $\operatorname{not}$ operations respectively. We have a homomorphism
$F$ defined as
\begin{align*}
    F (\cap) &= |,\\\
    F (\cup) &= \\&\\\
    F (\complement) &= \sim,\\\
    F (\emptyset) &= 0^n,\\\
    F (X^{\*}) &= 1^n,\\\
    F (\\{x_{j_1}, \ldots, x_{j_k}\\}) &= 0^n | h(x_{j_1}) | \cdots | h(x_{j_k}),
\end{align*}
where $h : X^{\*} \mapsto \\{0,1\\}^n$ is a cryptographic hash function, $s \in X^{\*}$ is a secret, and $\\{x_{j_1}, \ldots, x_{j_k}\\} \subseteq X^{\*}$.

The operations in $A$ are set operations, while the operations in $B$ are bitwise operations. For example, suppose $x,y \in \mathcal{P}(X^{\*})$ and $y \subset x$. If $F(x) = 0110$ and $F(y) = 0010$, then $F(x \land_A y) = F(x) \land_B F(y) = 0110 | 0010 = 00010 = F(y)$.

We assume $h$ uniformly distributes over $\\{0,1\\}^n$, which means that the apriori probability of a collision between two elements is $2^{-n}$. This is a fundamental property of cryptographic hash functions that we leverage in our construction.

If we map $A$ to $B$ using (approximate) homomorphism $F$ and then apply the same sequence of operations in both $A$ and $B$, we obtain some representation of the resultant set in $B$ for the ground truth in $A$. However, if we query $A$ and $B$ for membership of an element, we may find that there is a discrepancy between the results. We also find that the homomorphism $F$ itself is approximate in the negation operation, i.e., $F(\neg_A x) \neq \neg_B F(x)$.

These discrepenancies are a consequence of the finite number ($2^n$) of bit strings used to represent the elements $\mathcal{P}(X^{\*})$ in the Boolean algebra $A$. As a result, we say that the Boolean algebra $B$ is an approximate Boolean algebra when used to computationally model operations in $A$. The approximation error is a controllable parameter that we can adjust by changing the number of bits ($n$) in the representations. Different types of queries or operations are associated with different error rates, so we can adjust $n$ to suit the specific requirements of our system.

The cryptographic hash function $h$ is a one-way function that allows us to map elements from $X^{\*}$ to $\\{0,1\\}^n$ in a way that is computationally infeasible (even impossible, since the mapping is non-injective) to reverse. This property is crucial for the security of our construction, as it ensures that the original elements in $X^{\*}$ cannot be easily reconstructed from their bit string representations. Since $F$ uses $h$ in its construction, it is an (approximate) one-way homomorphism $F$ that maps Boolean algebra $A$ a Boolean algebra $B$ over (one-way) trapdoors.

### Homomorphism Properties

We have two Boolean algebras, $A$ and $B$ as previously described. We seek to model the operations in $A$ in $B$ using the homomorphism $F$. In the proofs to follow, let $X = \\{a,b\\}$ and two particular sets in $\mathcal{P}(X^{\*})$ be $x = \\{a,b,ab\\}$ and $y = \\{b,ab,bb\\}$.

To satisfy the properties of a homomorphism, the following properties must hold:

1. $F(x \land_A y) = F(x) \land_B F(y)$
2. $F(x \lor_A y) = F(x) \lor_B F(y)$
3. $F(\neg_A x) = \neg_B F(x)$
4. $F(0_A) = 0_B$
5. $F(1_A) = 1_B$

We show that these properties hold for all the properties except the third, which is not satisfied due to the approximate nature of the Boolean algebra over trapdoors. For this reason, we say that $F$ is an approximate Boolean algebra homomorphism. When we use the homomorphism $F$ to map elements from $A$ to $B$, we can perform operations in $A$ and have them correspond to operations in $B, albeit with some approximation error.

**Proof of First Property:** We seek to prove the identity $F(x \land_A y) = F(x) \land_B F(y)$ for the sets $x$ and $y$ defined above using the homomorphism $F$. Starting from the LHS, we have
\begin{align*}
F(x \land_A y)
    &= F(x \cap y) \\\
    &= F(\\{b,ab\\}) \\\
    &= 0^n | h(b) | h(ab).
\end{align*}

Now, let's start from the RHS and work our way to the same result:
$$
F(x) \land_B F(y) = \bigl(h(a) | h(b) | h(ab)\bigr) \\& \bigl(h(b) | h(ab) | h(bb)\bigr).
$$
Since $\\&$ and $|$ respectively denote the bitwise $\operatorname{and}$ and $\operatorname{or}$ operations, the result will be the bitwise $\operatorname{and}$ of all the bits in both groups of bitwise $\operatorname{or}$ operations. When we look at these two groups, we see that only $h(b)$ and $h(ab)$ occur in both, and so only the bits corresponding to $h(b)$ and $h(ab)$ will be set to $1$ in the result:
$$
F(x) \land_A F(y) = h(b) | h(ab).
$$
Thus, $F(x \land_A y) = F(x) \land_B F(y)$, which may be generalized to any two sets $x,y \in A$.
This completes the proof of the first property.

**Proof of Second Property:** We seek to prove the identity $F(x \lor_A y) = F(x) \lor_B F(y)$ for the sets $x$ and $y$ defined above using the homomorphism $F$. Starting from the LHS, we have
\begin{align*}
F(x \lor_A y)
    &= F(x \cup y) \\\
    &= F(\\{a,b,ab,bb\\}) \\\
    &= 0^n | h(a) | h(b) | h(ab) | h(bb).
\end{align*}

Now, let's start from the RHS and work our way to the same result:
$$
F(x) \lor_B F(y) = \bigl(0^n | h(a) | h(b) | h(ab)\bigr) | \bigl(0^n | h(b) | h(ab) | h(bb)\bigr).
$$
By the commutative and associative properties of the bitwise $\operatorname{or}$ operation, we can rewrite this as
$$
F(x) \lor_B F(y) = 0^n | 0^n | h(a) | h(b) | h(b) | h(ab) | h(ab) | h(bb).
$$
By the idempotent property of the bitwise $\operatorname{or}$ operation, we can simplify this to
$$
F(x) \lor_B F(y) = 0^n | h(a) | h(b) | h(ab) | h(bb).
$$
Thus, $F(x \lor_A y) = F(x) \lor_B F(y)$. This result generalizes to any two sets $x,y \in A$.
This completes the proof of the second property.

**Proof of Fourth Property:** We seek to prove the identity $F(0_A) = 0_B$.
The proof is trivial, as the empty set $0_A = \emptyset$ maps to the all-zero bit string $0_B = 0^n$ by definition of the homomorphism $F$: $F(0_A) = F(\emptyset) = 0^n = 0_B$. This completes the proof of the fourth property.

**Proof of Fifth Property:** We seek to prove the identity $F(1_A) = 1_B$.
The proof is also trivial, as the universal set $1_A = X^{\*}$ maps to the all-one bit string $1_B = 1^n$ by definition of the homomorphism $F$:

We have proven these properties for the homomorphism $F$ that maps the Boolean algebra $A$ to the Boolean algebra $B$ over trapdoors. However, note that we did not show identities like
$$
F(\\{a,b,ab\\} \land_A \\{b,ab,bb\\}) = F^{-1}(F(\\{a,b,ab\\}) \land_B F(\\{b,ab,bb\\}),
$$
since $F$ is one-way and has no inverse. Moreover, and related to this non-invertibility, we now show that the third property does not hold due to the properties of the cryptographic hash function $h$ used by the homomorphism $F$.

**Disproof of Third Property:** We seek to disprove the identity $F(\neg_A x) = \neg_B F(x)$ for the set $x$ defined above using the homomorphism $F$. Starting from the LHS, we have
\begin{align*}
F(\neg_A x)
    &= F(x^\complement) \\\
    &= F(X^{\*} \setminus \\{a,b,ab\\}),
\end{align*}
which is an infinite set that includes all elements in $X^{\*}$ except $a$, $b$, and $ab$. It includes elements like $bb$, $aaaaabbabababa$, $\epsilon$, and so on.

We assume $h$ uniformly distributes these elements over bit strings of length $n$. By the pihole principle, since there are only $2^n$ bit strings, every bit string in $\\{0,1\\}^n$ has multiple elements that hash to it, and thus when we apply $F$ to the complement of $x$,
$$
F(\neg_A x) = 0^n | h(bb) | h(aaaabbbabababa) | \cdots,
$$
the result is a bit string of all ones,
$$
F(\neg_A x) = 1^n.
$$

Now, let's start from the RHS and work our way to a decidedly different result:
$$
\neg_B F(x) = \sim \bigl(0^n | h(a) | h(b) | h(ab)\bigr).
$$
This is a finite number of elements bitwise $\operatorname{or}$-ed together and then its bitwise $\operatorname{not}$ computed. Due to the properties of the cryptographic hash function $h$, some bit positions will be set to $1$ and others set to $0$, resulting in a bit string that is not all ones:
$$
\neg_B F(x) \neq 1^n.
$$
Thus, $F(\neg_A x) \neq \neg_B F(x)$, which demonstrates that the third property does not hold. As a result, we say that $F$ is an approximate Boolean algebra homomorphism. As $|x|$ (size of $x$) increases, the probability that $F(\neg_A x) = 0^n$ goes to $1$, which is also true for $F(\neg_A x)$. Asymptotically, as $|x| \to \infty$, the third property holds. This completes the disproof of the third property.

## Single-Level Hashing Scheme

In the next section, `Two-Level Hashing Scheme`, we will introduce a two-level hashing scheme that reduces the space complexity of the single-level hashing scheme. In this section, we derive the space complexity of the single-level hashing scheme for representing free semigroups as "dense" bit strings of size $n$. We will find that, in order to keep the false positive rates on membership ($\in_B$) and subset ($\subseteq_B$) constant, the number of bits $n$ in the hash must grow exponentially with the number of elements in the set. This exponential growth limits the scalability of the scheme to very small sets. However, the two-level hashing scheme will address this limitation by reducing the space complexity for practical applications. We derive the space complexity of the single-level hashing scheme first because it is of theoretical interest and provides a foundation for the two-level hashing scheme.

### Relational Predicates

In this section, we view the Boolean algebra in a set-theoretic context. We then define some predicates that are fundamental to the algebra, namely membership and subset relations.

#### Set Membership

Set membership, denoted by $\in$, is a predicate that tests whether an element is in a set.
The Boolean algebra $A$ has a set membership predicate
$$
    \in_A : X^{\*} \times 2^{X^{\*}} \mapsto \mathrm{bool}
$$
defined as
$$
    a \in_A b := 1_b(a),
$$
where $1_b$ is the set indicator function.

For our approximate Boolean algebra $B$, the set membership predicate
$$
    \in_B : \\{0,1\\}^n \times \mathcal{P}(\\{0,1\\}^n) \mathrm{bool}
$$
is defined as
$$
    a \in_B b := a \land_B b = a.
$$

This just means we test for membership by testing that if $h(a)$ 
has a bit set (to 1), that bit must also be set in $h(b)$.
This permits false positives, i.e., $a \in_B b$ may test as true even if $a \notin_A b$.

When we take the union of two sets, we take the bitwise $\operatorname{or}$ of the two bit strings. 
First, let's consider two edge cases involving the empty set and the universal set.
Suppose we have two sets, the empty set $0_A = \emptyset$, which has the representation $F 0_B = 0^n$, and the singelton set $\\{a\\}$, which has the representation $F \\{a\\} = h(a)$. The union of these two sets is just the bitwise $\operatorname{or}$ of the two bit strings:
$$
    (F \emptyset) \lor_B (F \\{a\\}) = 0^n | h(a) = h(a).
$$
Similarly, if we have two sets, the universal set $X$, which has the representation $F X = 1^n$, and the singleton set $\\{a\\}$ as before, the union of these two sets is
$$
    (F X) \lor_B (F \\{a\\}) = 0^n | h(a) = 1^n.
$$

Now, let's consider the union of two singleton sets, $\\{a\\}$ and $\\{b\\}$,
$$
    (F \\{a\\}) \lor_B (F \\{b\\}) = h(a) | h(b).
$$

Let us denote the hash of $\\{a\\}$ as $Y$ and the hash of $\\{b\\}$ as $Z$. The probability that the $j$-th bit is set to $1$ in $Y$ is $1/2$ and the probability that the $j$-th bit is set to $1$ in $Z$ is also $1/2$. The probability that the $j$-th bit is set to $1$ in $X = Y \lor_B Z$ is the probability that the $j$-th bit is set to 1 in either $Y$ or $Z$:
\begin{align*}
\Pr\\{ X_j = 1\\}
    &= \Pr\\{Y_j | Z_j = 1\\}\\
    &= \Pr\\{(Y_j = 1) \lor (Z_j = 1)\\}.
\end{align*}

By De Morgan's laws, $A \lor B = \lnot (\lnot A \land \lnot B)$, so we may rewrite
the above as
\begin{align*}
\Pr\\{Y_j = 1 \lor Z_j = 1\\}
    &= \Pr\Bigr\\{\lnot  \bigl (\lnot (Y_j =1\bigr) \land \lnot (Z_j = 1)\bigr)\Bigr\\}\\\
    &= \Pr\Bigr\\{\lnot  \bigl( (Y_j = 0) \land (Z_j = 0)\bigr)\Bigr\\}\\\
    &= 1 - \Pr\Bigr\\{(Y_j = 0) \land (Z_j = 0)\Bigr\\}.
\end{align*}
By the independence of the two events, we may rewrite this as
$$
\Pr\\{Y_j = 1 \lor Z_j = 1\\} = 1 - \Pr\\{Y_j = 0\\} \Pr\\{Z_j = 0\\},
$$
By the assumption that the hash function is uniformly distributed over $\\{0,1\\}^n$, the probability that the $j$-th bit is set to 0 is $1/2$, so
$$
\Pr\\{Y_j = 1 \lor Z_j = 1\\} = 1 - 2^{-2}.
$$
We can generalize this result to $k$ union operations of unique singleton sets. The probability that the $j$-th bit is set to $1$ after taking their union is therefore given by
$$
    \Pr\\{X_j = 1\\} = 1 - 2^{-k}.
$$

We can simplify this result using the approximation $(1 - y) \approx e^{-y}$ for small $y$.
Here, $y = 2^{-k}$. Applying this approximation and simplifying, we get
$$
\Pr\\{X_j = 1\\} \approx e^{-1/2^k}.
$$

The probability that *all* of the bits are set to 1 is just the product of the probabilities that each of the bits is set to 1:
$$
    \Pr\\{X = 1^n\\} = \prod_{j=1}^{n} \Pr\\{X_j = 1\\} = \bigl(1 - 2^{-k}\bigr)^n \approx e^{-n/2^k}.
$$

Since $e^{-n/2^k} \to 1$ as $k \to \infty$, the union of $k$ singleton sets converges in probability to the universal set $1^n$. When we have the universal set, we have reached a stationary point in the union operation, and further unions will not change the result.

We use these results to compute the probability of a false positive in the membership and subset relations in the next section.

##### False Negatives and False Positives

Suppose that we have a set $W$ and we want to ask if a randomly chosen element $x \in X$ is a member of $W$. Let us denote the hash of $x$ as $h$ and the $j$-th bit as $h_j$. If $x$ actually is a member of $W$, then all of the bits where $F x$ is set to to $1$ will, by construction, be set to $1$ in $F W$. The probability of a **false negative** is $0$.

If $x$ is not a member of $W$, then a false positive may occur.
Recall that the bit string for $F W$ is denoted $W_j$ and the bit string for $F x$ is denoted $h_j$.
In order for a false positive to occur, it must be the case that if $h_j$ is set to $1$, then $W_j$ must also be set to $1$. Otherwise, if $h_j$ is set to $0$, then $W_j$ can be set to $0$ or $1$. This is just logical implication:
$$
    h_j = 1 \implies W_j = 1.
$$
We may rewrite logical implication as
$$
    \lnot ( h_j = 1 \land W_j = 0 ).
$$

We are interested in computing the probability of this event:
\begin{align*}
    \Pr\\{\lnot ( h_j = 1 \land W_j = 0)\\}
        &= 1 - \Pr\\{h_j = 1 \land W_j = 0\\} \\\
        &= 1 - \Pr\\{h_j = 1 \\} \Pr\\{W_j = 0\\}.
\end{align*}

Recall that we assume $h$ is uniformly distributed over $\\{0,1\\}^n$, so
$$
    \Pr\\{h_j = 1\\} = 1/2.
$$
From earlier, we showed that after $k$ union operations, the $j$-th bit is set to 0
with probability $2^{-k}$, i.e., $\Pr\\{W_j = 1\\} = 1 - 2^{-k}$. Therefore,
$$
\Pr\\{\lnot ( h_j = 1 \land W_j = 0)\\} = 1 - 2^{-(k+1)}.
$$
where $k$ is the number of elements in $W$.

For $x$ to be a false positive, this implication must hold for all $n$ bit positions:
\begin{align*}
\varepsilon
    &= \Pr\\{\text{$x \in W$ is a false positive}\\}\\\
    &= \prod_{j=1}^{n} \Pr\\{\lnot ( h_j = 1 \land W_j = 0)\\}\\\
    &= \prod_{j=1}^{n} (1 - 2^{-(k+1)})\\\
    &= (1 - 2^{-(k+1)})^n.
\end{align*}

###### Asymptotic False Positive Rate

Given the equation
$$
\varepsilon_{\in} = (1 - 2^{-(k+1)})^n,
$$
we can simplify it using the approximation $(1 - y) \approx e^{-y}$ for small $y$.
Here, $y = 2^{-(k+1)}$. Applying this approximation and simplifying, we get
$$
\varepsilon_{\in} \approx e^{-n 2^{-(k+1)}},
$$
which holds for even relatively small values of $k$. In Big-O notation, when we fix $n$, we have
$$
\varepsilon_{\in} = e^{\mathcal{O}(2^{-k})},
$$
which indicates that $\varepsilon$ approaches 1 exponentially fast as $k$ increases.

###### Space Complexity

Let's rewrite this where we show $n$ as a function of $\varepsilon$ and $k$:
$$
    n = \frac{\log \varepsilon}{\log(1 - 2^{-(k+1)})}.
$$
For small $x$, we can approximate $\log(1 - x) \approx -x$, thus
$$
    n \approx \frac{\log \varepsilon}{2^{-(k+1)}}.
$$
Holding $\varepsilon$ constant, we see that
$$
    n = \mathcal{O}(2^k),
$$
which means that to maintain a fixed error rate, the number of bits in the hash must grow exponentially with the number of elements in the set. This exponential growth limits the scalability of the scheme to very small sets.

We demonstrated that the probability of a false positive is a function of the number of elements $k$ in the set and the number of bits $n$ in the representation. Unsuprisingly, the probability of a false positive increases as the number of elements in the set $k$ increases and as the number of bits in the hash $n$ decreases, but now we have a probabilistic model that quantifies this relationship.

![False Positive Rate Analysis for Different Byte and Kilobyte Ranges](/image/combined_fpr_vs_size.png)

> In Figure 1, the false positive rate decreases exponentially as the byte size of the hash increases for smaller sets of elements (k=4 to k=10). In Figure 2, we observe the false positive rate behavior over larger kilobyte sizes for larger sets (k=12 to k=16). The green dashed line represents the 5% false positive rate threshold. As k increases, achieving this threshold requires significantly more space, highlighting the trade-off between set size and hash size.

#### Subset Relation

The subset relation has a predicate
$$
   \subseteq_A : 2^{X^{\*}} \mapsto 2^{X^{\*}} \mapsto \mathrm{bool}
$$
may be defined as
$$
    a \subseteq_A b := \forall x \in a, x \in b.
$$

In our approximate Boolean algebra $B$, the subset relation
$$
    \subseteq_B : \\{0,1\\}^n \times \\{0,1\\}^n \mapsto \\{0,1\\}
$$
is defined as
$$
   a \subseteq_B b := a | b = a,
$$
which is identical to the set-indicator function. However, they have different error probabilities.

Let us repurpose the earlier notation, letting $W$ be the bit string for $F W$ and $X$ be the bit string for $F X$. The probability that $W$ is a subset of $X$ is the probability that if $W_j = 1$, then $X_j = 1$ for $j=1,\ldots,n$. Each of these is just logical implication:
$$
    W_j = 1 \implies X_j = 1,
$$
which may be rerwitten as
$$
    \lnot ( W_j = 1 \land X_j = 0 ).
$$
The probability of this event is given by
\begin{align*}
    \Pr\\{\lnot ( W_j = 1 \land X_j = 0)\\}
        &= 1 - \Pr\\{W_j = 1 \land X_j = 0\\} \\\
        &= 1 - \Pr\\{W_j = 1 \\} \Pr\\{X_j = 0\\} \\\
        &= 1 - \Pr\\{W_j = 1 \\} \Pr\\{X_j = 0\\}.
\end{align*}
Recall that after $k$ union operations, the $j$-th bit in a set is $0$ with probability $2^{-k}$, therefore
\begin{align*}
\Pr\\{\lnot ( W_j = 1 \land X_j = 0)\\}
    &= 1 - \Pr\\{W_j = 1 \\} \Pr\\{X_j = 0\\} \\\
    &= 1 - (1 - 2^{-k_1}) 2^{-k_2},
\end{align*}
where $k_1$ and $k_2$ are the number of elements in $W$ and $X$ respectively.

For $W \subseteq X$ to be a false positive, this implication must hold for all $n$ bit positions:
\begin{align*}
\varepsilon
    &= \Pr\\{\text{$W \subseteq X$ is a false positive}\\}\\\
    &= \prod_{j=1}^{n} 1- (1 - 2^{-k_1}) 2^{-k_2}\\\
    &= \bigl(1 - (1 - 2^{-k_1}) 2^{-k_2}\bigr)^n.
\end{align*}

Recall that we may approximate $(1 - y) \approx e^{-y}$ for small $y$. Here,
$y = (1 - 2^{-k_1}) 2^{-k_2}$. Applying this approximation and simplifying, we get
$$
\varepsilon_{\subseteq} \approx e^{-n (1 - 2^{-k_1}) 2^{-k_2}},
$$
We can apply the approximation again to $1 - 2^{-k_1}$ to get
$$
\varepsilon_{\subseteq} \approx e^{-n e^{-(k_1 + k_2 \log 2)}}.
$$
In Big-O notation, when we fix $n$, we have
$$
\varepsilon_{\subseteq} = e^{\mathcal{O}(e^{-m})},
$$
where $m = k_1 + k_2 \log 2$. This indicates that $\varepsilon_{\subseteq}$ approaches 1 exponentially fast as $m$ increases for a fixed $n$. This result is consistent with the earlier analysis of the false positive rate for the membership relation.

Since equality can be written as $A \subseteq B \land B \subseteq A$, the false positive rate for the equality relation is the product of the false positive rates for the subset relations in both directions.

## Two-Level Hashing Scheme

In previous sections, we derived the space complexity of a single-level hashing scheme for
representing free semigroups as "dense" bit strings of size $n$. We found that, in order to keep the false positive rates on membership ($\in_B$) and subset ($\subseteq_B$) constant, the number of bits $n$ in the hash must grow exponentially with the number of elements in the set. This exponential growth limits the scalability of the scheme. To address this limitation, we introduce a two-level hashing scheme that reduces the space complexity for practical applications.

The two-level hashing scheme is a hierarchical structure that partitions the elements of the set into smaller subsets. For efficiency and compatability with existing hashing algorithms, when we hash an element, we do the following construction:

1. We have a hash function that outputs $q$ bits.
2. The first $w$ bits of the hash are used to determine the subset (bin) to which the element belongs, $2^w$ subsets in total.
3. The remaining $q-w$ bits of the hash are used to represent the element within the subset.
4. We have $2^w$ bins and $q-w$ bits for each bin, resulting in a total of $n = 2^w(q-w)$ bits.

This two-level hashing scheme allows us to reduce the number of bits in the hash for a given false positive rate $\varepsilon$. Let us derive the false positive rate for this construction, denoted as $\varepsilon(w,q)$.

### False Positive Rate

#### Membership Relation

Assume $k$ elements are in the set already, and we seek to test membership of an element $x$ in it.
False negatives do not occur, as before, but a false positive occurs if the following condition holds:

1. With probability $2^{-w}$, it maps to some particular subset.
2. In that subset, represented by $n-w$ bits, there are expected to be $k / 2^w$ elements.
3. We use the earlier derivation to find the probability of a false positive in the subset:

$$
\varepsilon(k,w,q) = (1 - 2^{-(k / 2^w + 1)})^{q-w}.
$$

Keeping $w$ and $q$ fixed, we have
$$
\varepsilon(k) = e^{\mathcal{O}(2^{-k})},
$$
which asymptotically is the same as the single-level hashing scheme. However, in practice, this is much more practical for reasonably large sets.

The space complexity of the two-level hashing scheme is given by $n = 2^w (q-w)$ bits for a false positive rate of $\varepsilon(k,w,q)$, or equivalently, $m = 2^w (q-w) / k$ bits per element for a false positive rate of $\varepsilon(k,w,q)$.

In Figure 3, we show the false positive rate for different values of $w$ and $q$.





## C++ Implementation: Single-Level Hashing Scheme

We can implement a C++ class that models the Boolean algebra over trapdoors.
We generalize the concept of trapdoors to any type $X$, which we also parameterize
over the size of the hash $N$.

Each `trapdoor<X,N>` represents a value of type `X` as a hash of size `N`. This
value is represented as a bit string of size `N` that is hashed using a cryptographic
hash function. It too a Bernolli Model of the equality operator, which returns a Bernoulli
Boolean that represents a Boolean value indicating if the trapdoors are equal with a false positive rate of $2^{-8 N}$ and a false negative rate of $0$. In our Bernoull Model notation, we say that this models the concept of
$$
B_{\mathrm{X' \times X' \mapsto \mathrm{bool}}}(=(X',X'))
$$
where $X'$ is a trapdoor of type $X$ and the latent value is the equality
predicate.


We model it as a Boolean algebra over bit strings, where the operations are the bitwise
$\operatorname{and}$, $\operatorname{or}$, and $\operatorname{not}$ operations. We also
define the equality operator, which returns a Bernoulli Boolean that represents a Boolean
value indicating if the trapdoors are equal with a false positive rate of $2^{-8 N}$,
and 


The `trapdoor_set<X,N>` class represents an approximate Boolean algebra over trapdoors (of type `X`), as we previously discussed. It is a specialization of `trapdoor<X,N>` that implements additional opoerations that can take place over the domain of `trapdoor<X,N>` and `trapdoor_set<X,N>`:

1. `empty_trapdoor_set<X,N>()` returns an empty set ($0_B$).
2. `universal_trapdoor_set<X,N>()` returns a universal set ($1_B$).
3. `operator+(trapdoor_set<X,N> const & x, trapdoor_set<X,N> const & y)` returns the union of two sets ($\lor_B$).
4. `operator~(trapdoor_set<X,N> const & x)` returns the complement of a set ($\neg_B$).
5. `operator*(trapdoor_set<X,N> const & x, trapdoor_set<X,N> const & y)` returns the intersection of two sets ($\land_B$).
7. `empty(trapdoor_set<X,N> const & xs)` returns a Bernoulli Boolean that represents a Boolean value that is true if the set is empty with a false positive rate. Essentially, it is the probability that the hash of the set is zero, $0_B$, which occurs with a false positive rate $\varepsilon = 2^{-8 N}$.
8. `in(trapdoor<X,N> const & x, trapdoor_set<X,N> const & xs)` returns a Bernoulli Boolean that represents a Boolean value that is true if the set contains the element with a false positive rate. See the section `Relational Predicates` in the `Single-Level Hashing Scheme` section for more details.

The class `trapdoor_set` is a template class that takes a type `X` and a size `N` as template arguments. The class has a `value_hash` member that is an array of `char` of size `N`. The class has a default constructor that initializes the `value_hash` to zero. The class has a copy constructor that is defaulted. The class has two static member functions `empty_trapdoor_set` and `universal_trapdoor_set` that return an empty set and a universal set, respectively. The class has three overloaded operators `+`, `!`, and `*` that implement the union ($\lor_B$), complement ($\lnot_B$), and intersection ($\land_B$) operations, respectively.

For closure, the class has a `hash` member function that returns a hash of the set, which is just the hash already stored in it. This means we can compose these operations to form more complex operations, like creating a powerset of `trapdoor_set` objects.

```cpp
#include <array>

template <typename X, size_t N>
struct trapdoor
{
    using value_type = X;

    /**
     * The constructor initializes the value hash to the given value hash.
     * Since the hash is a cryptographic hash, the hash is one-way and so we
     * cannot recover the original value from the hash.
     *
     * This also models the concept of an Oblivious Type, where the true value
     * is latent and we permit some subset of operations on it. In this case,
     * the only operation we permit is the equality and hashing operations.
     *
     * @param value_hash The value hash.
     */
    trapdoor(std::array<char, N> value_hash) : value_hash(value_hash) {}

    /**
     * The default constructor initializes the value hash to zero. This value
     * often denotes a special value of type X, but not necessarily.
     */
    trapdoor() { value_hash.fill(0); }

    std::array<char, N> value_hash;
};


/**
 * The hash function for the trapdoor class. It returns the value hash.
 */
template <typename X, size_t N>
auto hash(trapdoor const & x)
{
    return x.value_hash;
}

/**
 * Basic equality operator. It returns a Bernoulli Boolean that represents
 * a Boolean value indicating if the trapdoors are equal with a false positive rate
 * of 2^{-8 N}. Different specializes of the trapdoor class may have different
 * false positive rates, but this is a reasonable default value.
 */
template <typename X, size_t N>
auto operator==(trapdoor const & lhs, trapdoor const & rhs) const
{
    return bernoulli<bool>{lhs.value_hash == rhs.value_hash, -8*N};
}

/**
 * The `or` operation in the Boolean algebra over bit strings.
 */
template <typename X, size_t N>
auto lor(trapdoor lhs, trapdoor const & rhs)
{
    for (size_t i = 0; i < N; ++i)
        lhs.value_hash[i] |= rhs.value_hash[i];
    return lhs;
}

/**
 * The `and` operation in the Boolean algebra over bit strings.
 */
template <typename X, size_t N>
auto land(trapdoor lhs, trapdoor const & rhs)
{
    for (size_t i = 0; i < N; ++i)
        lhs.value_hash[i] &= rhs.value_hash[i];
    return lhs;
}

/**
 * The `not` operation in the Boolean algebra over bit strings.
 */
template <typename X, size_t N>
auto lnot(trapdoor x)
{
    for (size_t i = 0; i < N; ++i)
        lhs.value_hash[i] = ~lhs.value_hash[i];
    return lhs;
}

template <typename T>
auto minimum() { return std::numeric_limits<T>::min(); }

template <typename T>
auto maximum() { return std::numeric_limits<T>::max(); }

/**
 * The `minimum` operation in the Boolean algebra over trapdoors
 */
template <typename trapdoor<typename X, size_t N>>
auto minimum() { return trapdoor<X,N>(); }

/**
 * The `maximum` operation in the Boolean algebra over trapdoors
 */
template <typename trapdoor<typename X, size_t N>>
auto maximum()
{
    trapdoor<X,N> x;
    x.value_hash.fill(std::numeric_limits<char>::min())
    return x;
}
```


```cpp
/**
 * The trapdoor_set class models an approximate Boolean algebra over trapdoors.
 * It is a specialization of the trapdoor class that implements additional operations
 * that can take place over the domain of trapdoor and trapdoor_set.
 */
template <typename X, size_t N>
struct trapdoor_set: public trapdoor<X,N>
{
    trapdoor_set(
        double low_k = 0,
        double high_k = std::numeric_limits<double>::infinity()),
        std::array<char, N> value_hash)
        : trapdoor<trapdoor_set<X,N>,N>(value_hash),
          low_k(low_k),
          high_k(high_k) {}

    trapdoor_set() : trapdoor<trapdoor_set<X,N>,N>(), low_k(0), high_k(0) {}
    trapdoor_set(trapdoor_set const &) = default;
    trapdoor_set & operator=(trapdoor_set const &) = default;

    double low_k;
    double high_k;
};

/**
 * The size (cardinality) of the latent set (the set the trapdoor_set represents).
 *
 * @param xs The trapdoor_set.
 * @return The size range of the set.
 */
template <typename X, size_t N>
auto size(trapdoor_set<X,N> const & xs)
{
    return std::make_pair(xs.low_k, xs.high_k);
}

/**
The union operation.
 */
template <typename X, size_t N>
auto operator+(
    trapdoor_set<X,N> const & xs,
    trapdoor_set<X,N> const & ys)
{
    return trapdoor_set<X,N>(lor(xs, ys).value_hash,
        std::max(xs.low_k, ys.low_k), xs.high_k + ys.high_k);
}

template <typename X, size_t N>
void insert(trapdoor<X,N> const & x, trapdoor_set<X,N> & xs)
{
    for (size_t i = 0; i < N; ++i)
        xs.value_hash[i] |= x.value_hash[i];

    // since x could already be in xs, we do not increment the low_k
    // only the high_k
    xs.high_k += 1; 
}


template <typename X, size_t N>
auto operator~(trapdoor_set<X,N> x)
{
    for (size_t i = 0; i < N; ++i)
        x.value_hash[i] = ~x.value_hash[i];
    
    // assume |x| in [a, b]
    // then |~x| has the following analysis.
    //   - if |x| = a, then |~x| = |U| - a, where |U| is universal set
    //   - if |x| = b, then |~x| = |U| - b
    // so, |~x| in [|U|-b, |U|-a]
    x.low_k = maximum<X>() - x.high_k;
    x.high_k = maximum<X>() - x.low_k;
    return x;
}

template <typename X, size_t N>
auto operator*(
    trapdoor_set<X,N> const & x,
    trapdoor_set<X,N> const & y)
{
    return trapdoor_set<X>(x.value_hash & y.value_hash,
        0, // intersection could be empty
        std::min(x.high_k, y.high_k) // one could be a subset of the other
    );
}

template <typename X, size_t N>
bernoulli<bool> in(trapdoor<X,N> const & x, trapdoor_set<X,N> const & xs
{
    for (size_t i = 0; i < N; ++i)
        if ((x.value_hash[i] & xs.value_hash[i]) != x.value_hash[i])
            return bernoulli<bool>{true, 0};

    // earlier, we showed that the fp rate is (1 - 2^{-k})^n
    return bernoulli<bool>{false, };
}

/**
 * The subseteq_B predicate, x \subseteq_B y, is defined as x | y = x.
 * It has an false positive rate of eps = (1 - 2^{-(k_1 + k_2 log 2)})^n
 * which we approxiate as eps ~ e^{-n e^{-(k_1 + k_2 log 2)}}.
 * we store the log(eps) instead: -n e^{-(k_1 + k_2 log 2)}
 */
template <typename X>
bernoulli<bool> operator<=(
    trapdoor_set<X> const & x,
    trapdoor_set<X> const & y)
{
    for (size_t i = 0; i < N; ++i)
        if ((x.value_hash[i] & y.value_hash[i]) != x.value_hash[i])
        {
            return bernoulli<bool>(false, 0); // false negative rate is 0
        }

    // earlier, we showed that the fp rate is (1 - 2^{-(k_1 + k_2 log 2)})^n
    return bernoulli<bool>{true, };
    
}
```

```cpp
template <typename X>
bernoulli<bool> operator==(
    trapdoor_set<X> const & x
    trapdoor_set<X> const & y)
{
    for (size_t i = 0; i < N; ++i)
        if (x.value_hash[i] != y.value_hash[i])
            return bernoulli<bool>{false, 0.5};
}
```






## Appendix

### Marginal Uniformity

Suppose that we have a probability distribution over elements (unigrams) in $X^{\*}$ such that
$$
\Pr{}_D\\{a\\}
$$
is the probability of $a$ being generated by from some data generating process $D$. We can transform the homomorphism $F$ to mapping each $a \in X$ to multiple hashes inversely proportional to $\Pr{}_D(a)$ to satisfy the property
$$
\Pr\\{h(a)\\} \approx \Pr\\{h(b)\\}
$$
even if $\Pr{}_D\\{a\\}$ and $\Pr{}_D\\{b\\}$ are significantly different.

When we do a membership query, we uniformly sample one of these representations so that the unigram distribution of elements in $\\{0,1\\}$ is uniform. This is a kind of *marginal uniformity* that is a property of the transformation $F$.

However, this approach has serious shortcomings:

1. Only the marginal distribution of unigrams are uniformly distributed. Correlations in the joint distributions of the free semigroup $X^{\*}$, such as bigrams, are not accounted for. We can apply this transformation to larger sequences, but the space complexity grows exponentially with the length of the sequence for a fixed false positive rate. In practice, for large $X$, even the unigram model may need to be approximated due to space limitations.

2. When we apply the Boolean operations on an *untrusted system*, it cannot be given the information about the distribution of the elements in $X$. This means that Boolean operations like $\land$ and $\lor$ cannot be performed on the untrusted system, only relational querieslike $\in$ and $\subseteq$.