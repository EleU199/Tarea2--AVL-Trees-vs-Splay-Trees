#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

/**
 * @brief Splay Tree para claves uint32_t.
 *
 * Implementación orientada a experimentación:
 * - Usa un pool de nodos basado en std::vector.
 * - Los hijos y padres se almacenan como índices enteros.
 * - No almacena altura ni factor de balance.
 * - Toda búsqueda e inserción hace splay sobre el nodo accedido.
 */
class SplayTree {
public:
    using Key = std::uint32_t;
    using Index = std::int32_t;

    /**
     * @brief Constructor del Splay Tree.
     *
     * @param reserve_nodes Cantidad opcional de nodos a reservar inicialmente.
     */
    explicit SplayTree(std::size_t reserve_nodes = 0);

    /**
     * @brief Reserva memoria para cierta cantidad de nodos.
     *
     * @param n Cantidad de nodos a reservar.
     */
    void reserve(std::size_t n);

    /**
     * @brief Inserta una clave en el Splay Tree.
     *
     * Si la clave ya existe, incrementa su contador interno y hace splay
     * sobre el nodo encontrado.
     *
     * @param key Clave a insertar.
     * @return true si se creó un nodo nuevo, false si la clave ya existía.
     */
    bool insert(Key key);

    /**
     * @brief Busca una clave en el Splay Tree.
     *
     * Si la clave existe, hace splay sobre ella.
     * Si no existe, hace splay sobre el último nodo visitado.
     *
     * @param key Clave a buscar.
     * @return true si la clave está en el árbol, false en caso contrario.
     */
    bool search(Key key);

    /**
     * @brief Elimina todos los nodos del árbol.
     */
    void clear();

    /**
     * @brief Retorna la cantidad de nodos únicos almacenados.
     *
     * No cuenta repeticiones guardadas en count.
     */
    std::size_t size() const;

    /**
     * @brief Indica si el árbol está vacío.
     */
    bool empty() const;

    /**
     * @brief Calcula la altura actual del árbol.
     *
     * Se calcula recursivamente porque el Splay Tree no almacena altura.
     */
    int height() const;

    /**
     * @brief Verifica que el árbol cumpla la propiedad BST.
     */
    bool is_bst() const;

    /**
     * @brief Verifica que los enlaces parent sean consistentes.
     */
    bool has_valid_parents() const;

    /**
    * @brief Retorna las claves del árbol en recorrido preorden.
    *
    * Se usa para el experimento bonus Traversal Conjecture.
    * Implementación iterativa para evitar problemas de stack en árboles grandes.
    *
    * @return Vector con las claves visitadas en preorden.
    */
std::vector<Key> preorder_keys() const;
private:
    static constexpr Index NIL = -1;

    /**
     * @brief Nodo interno del Splay Tree.
     */
    struct Node {
        Key key;
        Index left;
        Index right;
        Index parent;
        std::uint32_t count;

        Node(Key key, Index parent);
    };

    Index root_;
    std::vector<Node> nodes_;

    /**
     * @brief Crea un nuevo nodo en el pool.
     *
     * @param key Clave del nuevo nodo.
     * @param parent Padre inicial del nuevo nodo.
     * @return Índice del nodo creado.
     */
    Index new_node(Key key, Index parent);

    /**
     * @brief Ejecuta splay sobre node hasta llevarlo a la raíz.
     *
     * Usa rotaciones Zig, Zag, Zig-Zig, Zag-Zag, Zig-Zag y Zag-Zig.
     *
     * @param node Nodo que será llevado a la raíz.
     */
    void splay(Index node);

    /**
     * @brief Rotación simple hacia la derecha.
     *
     * Corresponde a Zig.
     *
     * @param y Raíz del subárbol a rotar.
     * @return Nueva raíz del subárbol.
     */
    Index rotate_right(Index y);

    /**
     * @brief Rotación simple hacia la izquierda.
     *
     * Corresponde a Zag.
     *
     * @param x Raíz del subárbol a rotar.
     * @return Nueva raíz del subárbol.
     */
    Index rotate_left(Index x);

    /**
     * @brief Reemplaza el hijo old_child de parent por new_child.
     *
     * Función auxiliar para mantener correctamente los enlaces parent.
     */
    void replace_child(Index parent, Index old_child, Index new_child);

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
     * @brief Calcula recursivamente la altura de un subárbol.
     */
    int height_rec(Index node) const;

    /**
     * @brief Verifica recursivamente que los padres estén bien enlazados.
     */
    bool has_valid_parents_rec(Index node, Index expected_parent) const;
};


// =====================
// Implementación inline
// =====================

inline SplayTree::Node::Node(Key key, Index parent)
    : key(key),
      left(NIL),
      right(NIL),
      parent(parent),
      count(1) {}

inline SplayTree::SplayTree(std::size_t reserve_nodes)
    : root_(NIL), nodes_() {
    reserve(reserve_nodes);
}

inline void SplayTree::reserve(std::size_t n) {
    nodes_.reserve(n);
}

inline SplayTree::Index SplayTree::new_node(Key key, Index parent) {
    Index idx = static_cast<Index>(nodes_.size());
    nodes_.emplace_back(key, parent);
    return idx;
}

inline bool SplayTree::insert(Key key) {
    if (root_ == NIL) {
        root_ = new_node(key, NIL);
        return true;
    }

    Index current = root_;
    Index parent = NIL;

    while (current != NIL) {
        parent = current;

        if (key == nodes_[current].key) {
            nodes_[current].count++;
            splay(current);
            return false;
        }

        if (key < nodes_[current].key) {
            current = nodes_[current].left;
        } else {
            current = nodes_[current].right;
        }
    }

    Index inserted = new_node(key, parent);

    if (key < nodes_[parent].key) {
        nodes_[parent].left = inserted;
    } else {
        nodes_[parent].right = inserted;
    }

    splay(inserted);

    return true;
}

inline bool SplayTree::search(Key key) {
    Index current = root_;
    Index last_visited = NIL;

    while (current != NIL) {
        last_visited = current;

        if (key == nodes_[current].key) {
            splay(current);
            return true;
        }

        if (key < nodes_[current].key) {
            current = nodes_[current].left;
        } else {
            current = nodes_[current].right;
        }
    }

    if (last_visited != NIL) {
        splay(last_visited);
    }

    return false;
}

inline void SplayTree::replace_child(Index parent, Index old_child, Index new_child) {
    if (parent == NIL) {
        root_ = new_child;

        if (new_child != NIL) {
            nodes_[new_child].parent = NIL;
        }

        return;
    }

    if (nodes_[parent].left == old_child) {
        nodes_[parent].left = new_child;
    } else if (nodes_[parent].right == old_child) {
        nodes_[parent].right = new_child;
    }

    if (new_child != NIL) {
        nodes_[new_child].parent = parent;
    }
}

inline SplayTree::Index SplayTree::rotate_right(Index y) {
    if (y == NIL) {
        return NIL;
    }

    Index x = nodes_[y].left;

    if (x == NIL) {
        return y;
    }

    Index beta = nodes_[x].right;
    Index parent = nodes_[y].parent;

    replace_child(parent, y, x);

    nodes_[x].right = y;
    nodes_[y].parent = x;

    nodes_[y].left = beta;

    if (beta != NIL) {
        nodes_[beta].parent = y;
    }

    return x;
}

inline SplayTree::Index SplayTree::rotate_left(Index x) {
    if (x == NIL) {
        return NIL;
    }

    Index y = nodes_[x].right;

    if (y == NIL) {
        return x;
    }

    Index beta = nodes_[y].left;
    Index parent = nodes_[x].parent;

    replace_child(parent, x, y);

    nodes_[y].left = x;
    nodes_[x].parent = y;

    nodes_[x].right = beta;

    if (beta != NIL) {
        nodes_[beta].parent = x;
    }

    return y;
}

inline void SplayTree::splay(Index node) {
    if (node == NIL) {
        return;
    }

    while (nodes_[node].parent != NIL) {
        Index parent = nodes_[node].parent;
        Index grandparent = nodes_[parent].parent;

        if (grandparent == NIL) {
            // Zig
            if (nodes_[parent].left == node) {
                rotate_right(parent); 
            }
            // Zag
            else {
                rotate_left(parent);
            }
        }
        else if (nodes_[grandparent].left == parent &&
                 nodes_[parent].left == node) {
            // Zig-Zig
            rotate_right(grandparent);
            rotate_right(parent);
        }
        else if (nodes_[grandparent].right == parent &&
                 nodes_[parent].right == node) {
            // Zag-Zag
            rotate_left(grandparent);
            rotate_left(parent);
        }
        else if (nodes_[grandparent].left == parent &&
                 nodes_[parent].right == node) {
            // Zig-Zag
            rotate_left(parent);
            rotate_right(grandparent);
        }
        else {
            // Zag-Zig
            rotate_right(parent);
            rotate_left(grandparent);
        }
    }

    root_ = node;
}

inline void SplayTree::clear() {
    root_ = NIL;
    nodes_.clear();
}

inline std::size_t SplayTree::size() const {
    return nodes_.size();
}

inline bool SplayTree::empty() const {
    return root_ == NIL;
}

inline int SplayTree::height() const {
    return height_rec(root_);
}

inline int SplayTree::height_rec(Index node) const {
    if (node == NIL) {
        return 0;
    }

    int left_height = height_rec(nodes_[node].left);
    int right_height = height_rec(nodes_[node].right);

    return 1 + std::max(left_height, right_height);
}

inline bool SplayTree::is_bst() const {
    return is_bst_rec(root_, std::nullopt, std::nullopt);
}

inline bool SplayTree::is_bst_rec(
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

inline bool SplayTree::has_valid_parents() const {
    if (root_ == NIL) {
        return true;
    }

    if (nodes_[root_].parent != NIL) {
        return false;
    }

    return has_valid_parents_rec(root_, NIL);
}

inline bool SplayTree::has_valid_parents_rec(Index node, Index expected_parent) const {
    if (node == NIL) {
        return true;
    }

    const Node& current = nodes_[node];

    if (current.parent != expected_parent) {
        return false;
    }

    return has_valid_parents_rec(current.left, node) &&
           has_valid_parents_rec(current.right, node);
}

inline std::vector<SplayTree::Key> SplayTree::preorder_keys() const {
    std::vector<Key> sequence;
    sequence.reserve(nodes_.size());

    if (root_ == NIL) {
        return sequence;
    }

    std::vector<Index> stack;
    stack.reserve(nodes_.size());
    stack.push_back(root_);

    while (!stack.empty()) {
        Index node = stack.back();
        stack.pop_back();

        sequence.push_back(nodes_[node].key);

        if (nodes_[node].right != NIL) {
            stack.push_back(nodes_[node].right);
        }

        if (nodes_[node].left != NIL) {
            stack.push_back(nodes_[node].left);
        }
    }

    return sequence;
}