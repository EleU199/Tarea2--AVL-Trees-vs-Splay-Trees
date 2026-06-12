#pragma once

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <vector>

/**
 * @brief Árbol AVL para claves uint32_t.
 *
 * Implementación orientada a experimentación:
 * - Usa un pool de nodos basado en std::vector.
 * - Los hijos se almacenan como índices enteros, no como punteros.
 * - Mantiene altura en cada nodo para calcular el factor de balance.
 */
class AVLTree {
public:
    using Key = std::uint32_t;
    using Index = std::int32_t;

    /**
     * @brief Constructor del árbol AVL.
     *
     * @param reserve_nodes Cantidad opcional de nodos a reservar inicialmente.
     */
    explicit AVLTree(std::size_t reserve_nodes = 0);
    /**
     * @brief Reserva memoria para cierta cantidad de nodos.
     *
     * @param n Cantidad de nodos a reservar.
     */
    void reserve(std::size_t n);
    
    /**
     * @brief Inserta una clave en el árbol.
     *
     * Si la clave ya existe, se incrementa su contador interno.
     *
     * @param key Clave a insertar.
     * @return true si se creó un nodo nuevo, false si la clave ya existía.
     */
    bool insert(Key key);

    /**
     * @brief Busca una clave en el árbol.
     *
     * En AVL, la búsqueda no modifica la estructura.
     *
     * @param key Clave a buscar.
     * @return true si la clave está en el árbol, false en caso contrario.
     */
    bool search(Key key) const;

    /**
     * @brief Elimina todos los nodos del árbol.
     */
    void clear();

    /**
     * @brief Retorna la cantidad de nodos únicos almacenados.
     *
     * No cuenta repeticiones.
     */
    std::size_t size() const;

    /**
     * @brief Indica si el árbol está vacío.
     */
    bool empty() const;

    /**
     * @brief Retorna la altura del árbol.
     */
    int height() const;

    /**
     * @brief Verifica que el árbol cumpla la propiedad BST.
     *
     * Función útil para debug y tests.
     */
    bool is_bst() const;

    /**
     * @brief Verifica que el árbol cumpla el invariante AVL.
     *
     * Revisa que para todo nodo se cumpla |BF| <= 1.
     */
    bool is_avl() const;

private:
    static constexpr Index NIL = -1;

    /**
     * @brief Nodo interno del árbol AVL.
     */
    struct Node {
        Key key;
        Index left;
        Index right;
        int height;
        std::uint32_t count;

        explicit Node(Key key);
    };

    Index root_;
    std::vector<Node> nodes_;

    /**
     * @brief Crea un nuevo nodo en el pool.
     *
     * @param key Clave del nuevo nodo.
     * @return Índice del nodo creado.
     */
    Index new_node(Key key);

    /**
     * @brief Inserta recursivamente una clave desde un subárbol.
     *
     * @param node Índice de la raíz del subárbol.
     * @param key Clave a insertar.
     * @param inserted_new_node Queda en true si se creó un nodo nuevo.
     * @return Nueva raíz del subárbol luego de insertar y balancear.
     */
    Index insert_rec(Index node, Key key, bool& inserted_new_node);

    /**
     * @brief Retorna la altura de un nodo.
     *
     * Para NIL retorna 0.
     */
    int node_height(Index node) const;

    /**
     * @brief Actualiza la altura de un nodo según sus hijos.
     */
    void update_height(Index node);

    /**
     * @brief Calcula el factor de balance de un nodo.
     *
     * BF = altura(izquierdo) - altura(derecho)
     */
    int balance_factor(Index node) const;

    /**
     * @brief Balancea un nodo AVL si queda desbalanceado.
     *
     * Maneja los casos LL, RR, LR y RL.
     *
     * @param node Índice del nodo posiblemente desbalanceado.
     * @return Nueva raíz del subárbol balanceado.
     */
    Index balance(Index node);

    /**
     * @brief Rotación simple hacia la derecha.
     *
     * Corresponde a Zig.
     */
    Index rotate_right(Index y);

    /**
     * @brief Rotación simple hacia la izquierda.
     *
     * Corresponde a Zag.
     */
    Index rotate_left(Index x);

    /**
    * @brief Verificación recursiva de propiedad BST.
    *
    * min_key y max_key son cotas estrictas.
    */
    bool is_bst_rec(
        Index node,
        std::optional<Key> min_key,
        std::optional<Key> max_key
    ) const;

    /**
    * @brief Verificación recursiva de invariante AVL.
    */
    bool is_avl_rec(Index node) const;
};



// Implementación inline de funciones simples


inline AVLTree::Node::Node(Key key)
    : key(key),
      left(NIL),
      right(NIL),
      height(1),
      count(1) {}

inline AVLTree::AVLTree(std::size_t reserve_nodes)
    : root_(NIL), nodes_() {
    reserve(reserve_nodes);
}

inline void AVLTree::reserve(std::size_t n) {
    nodes_.reserve(n);
}

inline bool AVLTree::insert(Key key) {
    bool inserted_new_node = false;

    root_ = insert_rec(root_, key, inserted_new_node);

    return inserted_new_node;
}
inline AVLTree::Index AVLTree::insert_rec(Index node, Key key, bool& inserted_new_node) {
    // Caso base: llegamos a una posición vacía.
    if (node == NIL) {
        inserted_new_node = true;
        return new_node(key);
    }

    // Inserción clásica de ABB.
    if (key < nodes_[node].key) {
        nodes_[node].left = insert_rec(nodes_[node].left, key, inserted_new_node);
    } 
    else if (key > nodes_[node].key) {
        nodes_[node].right = insert_rec(nodes_[node].right, key, inserted_new_node);
    } 
    else {
        // Clave repetida: no creamos nodo nuevo.
        nodes_[node].count++;
        inserted_new_node = false;
        return node;
    }

    // Al volver de la recursión, actualizamos altura.
    update_height(node);

    // Luego balanceamos si quedó con BF = 2 o BF = -2.
    return balance(node);
}

inline AVLTree::Index AVLTree::new_node(Key key) {
    Index idx = static_cast<Index>(nodes_.size());
    nodes_.emplace_back(key);
    return idx;
}

inline int AVLTree::node_height(Index node) const {
    if (node == NIL) {
        return 0;
    }

    return nodes_[node].height;
}

inline void AVLTree::update_height(Index node) {
    if (node == NIL) {
        return;
    }

    int left_height = node_height(nodes_[node].left);
    int right_height = node_height(nodes_[node].right);

    nodes_[node].height = 1 + std::max(left_height, right_height);
}

inline int AVLTree::balance_factor(Index node) const {
    if (node == NIL) {
        return 0;
    }

    return node_height(nodes_[node].left) - node_height(nodes_[node].right);
}

inline AVLTree::Index AVLTree::balance(Index node) {
    if (node == NIL) {
        return NIL;
    }

    int bf = balance_factor(node);

    // Caso LL o LR: desbalance hacia la izquierda.
    if (bf > 1) {
        Index left_child = nodes_[node].left;

        // Caso LR: primero rotamos el hijo izquierdo hacia la izquierda.
        if (balance_factor(left_child) < 0) {
            nodes_[node].left = rotate_left(left_child);
        }

        // Caso LL: rotación simple hacia la derecha.
        return rotate_right(node);
    }

    // Caso RR o RL: desbalance hacia la derecha.
    if (bf < -1) {
        Index right_child = nodes_[node].right;

        // Caso RL: primero rotamos el hijo derecho hacia la derecha.
        if (balance_factor(right_child) > 0) {
            nodes_[node].right = rotate_right(right_child);
        }

        // Caso RR: rotación simple hacia la izquierda.
        return rotate_left(node);
    }

    return node;
}

inline AVLTree::Index AVLTree::rotate_right(Index y) {
    if (y == NIL) {
        return NIL;
    }

    Index x = nodes_[y].left;

    if (x == NIL) {
        return y;
    }

    Index beta = nodes_[x].right;

    // Rotación:
    //        y                 x
    //       / \               / \.
    //      x   C    --->     A   y
    //     / \                   / \.
    //    A   B                 B   C
    nodes_[x].right = y;
    nodes_[y].left = beta;

    // Primero actualizamos y, porque ahora y quedó debajo de x.
    update_height(y);

    // Luego actualizamos x, que ahora es la nueva raíz del subárbol.
    update_height(x);

    return x;
}

inline AVLTree::Index AVLTree::rotate_left(Index x) {
    if (x == NIL) {
        return NIL;
    }

    Index y = nodes_[x].right;

    if (y == NIL) {
        return x;
    }

    Index beta = nodes_[y].left;

    // Rotación:
    //      x                     y
    //     / \                   / \.
    //    A   y      --->       x   C
    //       / \               / \.
    //      B   C             A   B
    nodes_[y].left = x;
    nodes_[x].right = beta;

    // Primero actualizamos x, porque ahora x quedó debajo de y.
    update_height(x);

    // Luego actualizamos y, que ahora es la nueva raíz del subárbol.
    update_height(y);

    return y;
}

inline bool AVLTree::search(Key key) const {
    Index current = root_;

    while (current != NIL) {
        const Node& node = nodes_[current];

        if (key == node.key) {
            return true;
        }

        if (key < node.key) {
            current = node.left;
        } else {
            current = node.right;
        }
    }

    return false;
}

inline void AVLTree::clear() {
    root_ = NIL;
    nodes_.clear();
}

inline bool AVLTree::empty() const {
    return root_ == NIL;
}

inline std::size_t AVLTree::size() const {
    return nodes_.size();
}

inline int AVLTree::height() const {
    return node_height(root_);
}

inline bool AVLTree::is_bst() const {
    return is_bst_rec(root_, std::nullopt, std::nullopt);
}

inline bool AVLTree::is_avl() const {
    return is_avl_rec(root_);
}

inline bool AVLTree::is_bst_rec(
    Index node,
    std::optional<Key> min_key,
    std::optional<Key> max_key
) const {
    if (node == NIL) {
        return true;
    }

    const Node& current = nodes_[node];

    if (min_key.has_value() && current.key <= min_key.value()) {
        return false;
    }

    if (max_key.has_value() && current.key >= max_key.value()) {
        return false;
    }

    return is_bst_rec(current.left, min_key, current.key) &&
           is_bst_rec(current.right, current.key, max_key);
}

inline bool AVLTree::is_avl_rec(Index node) const {
    if (node == NIL) {
        return true;
    }

    const Node& current = nodes_[node];

    int left_height = node_height(current.left);
    int right_height = node_height(current.right);

    int bf = left_height - right_height;

    if (bf < -1 || bf > 1) {
        return false;
    }

    int expected_height = 1 + std::max(left_height, right_height);

    if (current.height != expected_height) {
        return false;
    }

    return is_avl_rec(current.left) && is_avl_rec(current.right);
}