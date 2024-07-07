# Boolean Ring Over Trapdoors

Say we have the Boolean ring
$$
A := \Bigl(\mathcal{P}(\{0,1\}^*), \triangle, \cap, \complement, \emptyset\Bigr)
$$
where $\{0,1\}^*$ is the free semigroup over $\{0,1\}$, which is the closure over concatenation
$$
\# : \{0,1\}^* \times \{0,1\}^* \mapsto \{0,1\}^*.
$$

Consider the Boolean ring
$$
B := \Bigl(\{0,1\}^m, \oplus, \land, \mathrm{id}, 0^m\Bigr)
$$
and suppose we wish to provide a homomorphism of type $A \mapsto B$ that *only* preserves identities.

## Definition

The homomorphism $\operatorname{G} : A \mapsto B$ is defined as
$$
\operatorname{G}(\beta) :=
\begin{cases}
\quad h(x) & \qquad \beta \in {\{0,1\}^*} \\
\quad \oplus & \qquad \beta = \triangle \\
\quad \land & \qquad \beta = \cap \\
\quad \operatorname{id} & \qquad \beta = \complement \\
\quad 0^m & \qquad \beta = \emptyset
\end{cases}
$$
where $\oplus$, $\land$, and $\mathrm{id}$ are respectively bitwise *exclusive-or*, bitwise *and*, and *identity*.

This homomorphism is due to the presence of the hash function $h : {\{0,1\}^*} \mapsto {\{0,1\}^m}$ for mapping sets to bit strings.

The identity relation
$$
(\operatorname{G}(\mathcal{A}) \ \operatorname{F}(\triangle) \ \operatorname{G}(\mathcal{B})) = \operatorname{G}(\mathcal{A} \triangle \mathcal{B})
$$
is guaranteed by the homomorphism.[^1]

[^1]: Observe that if $\mathcal{A}$ and $\mathcal{B}$ are disjoint, $\mathcal{A} \triangle \mathcal{B} = \mathcal{A} \cup \mathcal{B}$.

There is an *isomorphism* between $\operatorname{G}$ and $\operatorname{F}$ since $\mathcal{A} \cup \mathcal{B} := (\mathcal{A} \triangle \mathcal{B}) \triangle (\mathcal{A} \cap \mathcal{B})$. If the trusted system applies only the *group*
$$
D := \Bigl(\{0,1\}^m, \oplus, \operatorname{id}, 0^m\Bigr)
$$
the number of $1$'s and the cardinality of the set has no relation.

Given a set $\{x,y,z\} = \{x\} \cup \{y\} \cup \{z\}$, $\operatorname{G}$ maps this to $h(x) \oplus h(y) \oplus h(z)$. Since the hash function $h$ is cryptographic, by the property of the exclusive-or operation each set in $A$ maps to a bit string in $\{0,1\}^m$ that appears random. That is to say, it is indistinguishable from random noise. However, it only supports *identity* relations. The computational basis of this Boolean ring over trapdoors is the union operation and the equality predicate, and other functions like membership, intersection, or complement are not supported. (If complement was supported, then by De Morgan's laws, intersection would be supported as well.)

## Theorem

False positives on the *equality* predicate occur with probability
$$
\varepsilon(m) = 2^{-m}
$$
where $m$ is the bit length of the hash function in the Boolean cipher ring.

### Proof

Two sets are said to be identical if they map to the same bit string of size $m$. By the properties of the hash function, this occurs with probability $2^{-m}$:
$$
\varepsilon = \Pr\{\text{a collision occurs}\} = \frac{1}{\text{\#(bit strings of length $n$)}} = \frac{1}{2^m}.
$$

which occurs with probability $2^{-m}$ since each set maps to a random bit string of size $m$.
