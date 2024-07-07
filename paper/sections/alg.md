# Cipher Boolean algebras

TODO: mention the order of the model. xor-set is a higher-order model on equality. No easy solution there.
The other union set one is a second-order $\pm$ model, I believe.

*Cipher* value types may come in many shapes.
They may be parametric types, e.g., $\mathcal{T}$ is a cipher type of plain type $T$ and they may provide differing levels of *confidentiality*, whether as a simple substitution cipher, homophonic cipher, or homomorphic encryption.

In what follows, we consider several ciphers sets, one in which is a Boolean algebra that provides for all set relations like *membership*, *identity*, and *subset*, and the other is a group over symmetric difference and intersection but only offers the *identity* relation (or only one of its representations in a, say, homophonic encryption system). Both, however, model their relations as predicate functions which return *plaintext* Booleans rather than cipher variations, i.e., $\{0,1\}$ as opposed to the more confidential $\mathcal{\{0,1\}}$.

NOTE: If using something like homophonic encryption, then identity is no longer maintained. A cipher set $\mathcal{A}$ sent in one round may be different and is *incomparable* to a cipher set $\mathcal{A}$ sent in another round.

NOTE 2: When, say, including bigrams (for whatever reason), if we want to be able to construct a cipher bigram from two cipher elements on the untrusted system, then in the cipher map or set on the untrusted system, all of these combinations must be included separately (unless rate-distorted, in which case errors). Since this may limit its value, we cannot support these operations and just go ...

Suppose we have the Boolean algebra

$$
A := \left(\mathcal{P}(\{0,1\}^*), \cup, \cap, \complement, \emptyset, \{0,1\}^*\right)
$$

Since $\{0,1\}^*$ can encode any tuple of elements, and $\mathcal{P}(\{0,1\}^*)$ can encode any sum type of tuples, this Boolean algebra may be a reasonable model for hashing algebraic data types, while still permitting some operations on the resultant hash.

Consider the Boolean algebra

$$
B := \left(\{0,1\}^m, \lor, \land, \neg, 0^m, 1^m\right)
$$

and suppose we wish to provide a homomorphism of type $A \mapsto B$ that, approximately, preserves relations while *obscuring* or *encrypting* the identities.

A *cryptographic hash function* is given by the following definition.

## Definition

The *hash function* $\mathrm{hash} \colon \{0,1\}^* \mapsto \{0,1\}^m$ maps (hashes) bit strings of arbitrary-length to bit strings of fixed-length $m$ and approximates a *random oracle* over its codomain.

## Definition

The homomorphism $\mathrm{F} \colon A \mapsto B$ is defined as

$$
\mathrm{F}(\beta) :=
\begin{cases} 
	\quad \mathrm{hash}(\beta) & \qquad \beta \in \{0,1\}^* \\
	\quad \lor & \qquad \beta = \cup \\
	\quad \land & \qquad \beta = \cap \\
	\quad \neg & \qquad \beta = \complement \\
	\quad 0^m & \qquad \beta = \emptyset \\
	\quad 1^m & \qquad \beta = \{0,1\}^*
\end{cases}
$$

where $\lor$, $\land$, and $\neg$ are respectively bitwise *or*, *and*, and *negation*.

This homomorphism is *one-way* for two independent reasons. First, $\mathrm{F}$ is not injective, i.e., multiple values in $\{0,1\}^*$ (countably infinite many, since $\{0,1\}^*$ is countably infinite) must map to at least one value in $\{0,1\}^m$. Second, the hash function $\mathrm{hash}$ is a cryptographic hash function that is resistant to preimage attacks, i.e., given a $y$ it is *hard* to find an $x$ such that $\mathrm{hash}(x) = y$ but given an $x$ it is *easy* to find a $y$ such that $\mathrm{hash}(x) = y$.

Thus, given a set $\{x,y,z\} = \{x\} \cup \{y\} \cup \{z\}$, $\mathrm{F}$ maps this to $\mathrm{hash}(x) \lor \mathrm{hash}(y) \lor \mathrm{hash}(z)$.

These model *positive approximate sets* with a false positive rate given by the following theorem.

## Theorem

The false positive rate is a function of $m$ and $n$ and is given by

$$
\varepsilon(m,n) = \alpha^m(n)
$$
where
$$
\alpha(n) := \left(1-2^{-(n+1)}\right)
$$

## Proof

Proof here. See paper sheets that are star-numbered.

The *bit-rate*, the bits per element, is given by the following corollary.

## Corollary

$$
\mathrm{b}(n,\varepsilon) = \frac{\log_2 \varepsilon}{ n \alpha(n)}
$$

## Proof

Proof here. See paper sheets that are star-numbered.

The *partial derivative* of $\mathrm{b}$ with respect to $\varepsilon$ is given by

$$
\frac{\partial \mathrm{b}}{\partial \varepsilon} = -\frac{\mathrm{b}}{\mathrm{B}(\varepsilon)}
$$

where $\mathrm{B}(\varepsilon) := -\varepsilon \log_2 \varepsilon$ is the *binary entropy function*. This partial is less than zero everywhere but finds its minimum at $\frac{1}{e}$.

The *absolute efficiency* of this homomorphism, when implemented as a data structure for a false positive rate $\varepsilon$ is given by

$$
\operatorname{AE}(n) = -n \log_2 \alpha(n)
$$

which is exponential with respect to $n$.

As $n \to \infty$, $\operatorname{AE}(n)$ goes to $0$. It is only practical for small values of $n$, where $n$ is the number of elements of the set.

The *identity* relation has false positives given by the following.
