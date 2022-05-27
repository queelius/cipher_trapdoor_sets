/**
 * flattened_trapdoor<X>
 */


#pragma once

#include <string_view>
#include <functional>
using std::string_view;

/**
 * random_trapdoor<X> is an irregular type.
 *
 * A random trapdoor of X is a one-way transformation of values of type X
 * to random_trapdoor<X>. Let the one-way transformation be denoted by
 *     make_random_trapdoor : [Secret] -> X -> [Seed] -> random_trapdoor<X>
 * where the first argumen is a secret key, the second argument is the value
 * of type X to map to random_trapdoor<X>, and the third argument is a seed
 * value to facilitate pseudo-random sampling.
 * 
 * make_random_trapdoor is one-way in two independent senses:
 * 
 *     (1) It is easy to compute, but going in the oppose direction, from
 *         random_trapdoor<X> to X, is hard. That is, if we only have
 *         random_trapdoor<X>{x}, it is hard to find any value y that
 *         maps to random_trapdoor<X>{x}. This is called preimage resistance.
 * 
 *     (2) Many values of type X may be mapped to a single value of type
 *         random_trapdoor<X>. That is, make_random_trapdoor is a homomorphism
 *         where information is irrevocably lost going from X to
 *         random_trapdoor<X>.
 * 
 * make_random_trapdoor(s,x,N), where N is random natural number, is a discrete
 * random variable over some set of byte strings (which represents a
 * random_trapdoor<X>), where the byte strings are uncorrelated with x when s is
 * unknown. 
 * 
 * Consider the random sample
 *     make_random_trapdoor(s,W1,N1),...,make_random_trapdoor(s,Wk,Nk)
 * where Wj is a random variable over X and Nj is a discrete uniform random
 * variable over the bounded natural numbers.
 * 
 * Assuming i.i.d., the sample with maximum entropy is obtained when for each
 * x in X, make_random_trapdoor(s,x,N) has a support of size n(x) := P(x)/P(y)
 * where y is the value such that P(y) is minimized.
 * 
 * Example:
 * Suppose P(x) = .1 and minimum probability P(y) = .001.
 * Then n(x) maps to 100. If P(y) is negligible compared to P(x), this can
 * result in an explosion of homophonic substitutions, which may be problematic.
 * 
 * Instead, we can compose n with some monotonically increasing transformation
 * that maps large values to smaller values, such as
 *     w(n) := floor(1 + n),
 * e.g., (w . n)(x) = 7.
 * 
 * Or, alternatively to or in conjunction with, we may give all elements with
 * a probability less than q a single representation, and then use
 *     n(x) := P(x)/q.
 * 
 * If the values in the random sample are not independently distributed, then
 * this construction will show patterns in the sample, e.g., given that the
 * previous value was x' the probability that the next value is y' may be
 * different than the marginal distribution of y'. In this case, we wish to
 * generate random trapdoors from a conditional probability distribution. The
 * more values the conditional distribution can entertain, the fewer
 * correlations and patterns in the homophonic substitution cipher. However,
 * this significantly complicates things.
* 
 * 
 * Note that flattened_trapdoor<X> is an *approximate cipher value type* since the value
 * of a t of type flattened_trapdoor<X> is difficult, if not impossible, to
 * decode (decipher) to its objective value of type X.
 * 
 * It has the further curious property of being *ambiguous* in that
 * T(x)
 * 
 * Furthermore, since == : trapdoor<X> -> trapdoor<X> -> bool (and its
 * complement) can be trivially defined (note that a cipher map could be
 * separately constructed for == which is not approximate), trapdoor<X>
 * is an *approximate* value type with respect to == and !=.
 * 
 * 
 * Few operations for this type can be defined on the basis of the hash
 * values alone. Cipher maps may be used to operate on these trapdoors more
 * generally, but they require access to the preimage hash values, i.e.,
 * either they require U(t) (which is expensive and is not one-to-one) or
 * the cipher maps were generated by someone who has access to the secret.
 * 
 * 
 */



template <typename X>
struct trapdoor
{
    using value_type = X;

    static constexpr auto VALUE_BYTE_LENGTH = sizeof(value_hash);
    static constexpr auto VALUE_BIT_LENGTH = CHAR_BIT * VALUE_BYTE_LENGTH;

    auto & operator=(trapdoor const & rhs)
    {
        value_hash = rhs.value_hash;
        key_hash = rhs.key_hash;
    }

    size_t value_hash;

    // the key hash is a hash of the secret key,
    // which faciliates a form of dynamic type checking.
    size_t key_hash;
};

template <typename X, size_t N>
struct constant_fn
{
    static auto operator()(X const &) const { return N; };
}

template <
    typename X,
    typename N = constant<X,1>,
    template <typename> typename H = std::hash
>
struct random_trapdoor_generator
{
    random_trapdoor<X> operator()(X const & x)
    {
        auto n = n(x);
        auto s = seed;
        hash_combine(s,x);
        hash_combine(s,n);

        // pack into 4 bytes
        return random_trapdoor<X>{s};
    }

    template<typename T>
    void hash_combine(size_t & seed, T const & x)
    {
        seed ^= H<T>{}(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    H h;
    P p;
    string_view k,
    size_t seed
}

namespace std
{
    template <typename X>    
    struct hash<trapdoor<X>>
    {
        size_t operator()(trapdoor<X> const & x)
        {
            return x.value_hash;
        }
    }
}


/**
 * Given a shared secret, the random error on equality on trapdoor<X>, which is
 * a simple one-way substitution cipher where representational equality implies
 * equality, follows a second-order, positive Bernoulli model over Boolean
 * values with a false positive rate
 *     2^-bit_length(trapdoor<X>::hash_value).
 * 
 * Proof:
 * 
 * Suppose we have two objects of type X, denoted by A and B, where X is a
 * regular type such that A == B => hash(A) == hash(B).
 * 
 * Suppose A == B. Then, by definition, hash(A) == hash(B) and thus
 * trapdoor<X>{A} and trapdoor<X>{B} have the same representation where
 * representational equality implies equality. Thus,
 *     P[trapdoor<X>{A} == trapdoor<X>{B} | A == B] = 1.
 * 
 * Suppose A != B. Assume the hash function models a random hash function.
 * Since there are 2^bit-length(trapdoor<X>::hash_value) possible bit patterns
 * in the hashes of A and B, the probability that hash(A) and hash(B) collide is
 *     2^-bit-length(trapdoor<X>::hash_value).
 * Thus,
 *     P[trapdoor<X>{A} == trapdoor<X>{B} | A != B] =
 *         2^-bit_length(trapdoor<X>::hash_value).
 * 
 * Taking the two probabilities above together, we see that the true equality
 * is true, the probability of error is 0, and when the true equality is false,
 * the probability of error is 2^-bit_length(trapdoor<X>::hash_value).
 */
template <typename X>
auto operator==(trapdoor<X> const & x, trapdoor<X> const & y)
{
    return bernoulli<bool,2>
    {
        // realized value; may be erroneous
        x.value_hash == y.value_hash && x.key_hash == y.key_hash,
        // if truely true, then expected error is 0
        0.,
        // if truely false, then expected error is 2^-k where k is bit
        // length of hash value. (we ignore collisions on the secret.)
        std::pow(2.,-(double)trapdoor<X>::HASH_VALUE_BIT_LENGTH)
    };
}

template <typename X>
bool operator!=(trapdoor<X> const & x, trapdoor<X> const & y)
{
    return !(x == y);
}
