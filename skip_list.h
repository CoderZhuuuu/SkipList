#pragma once

#include <random>
#include <memory>
#include <numeric>

/* Constants */
namespace skiplist {

	const int MAX_LEVEL = 32;

}

/* Comparator */
namespace skiplist {

	class ComparatorBase {
	protected:
		virtual ~ComparatorBase() {}
	};

	class DefaultComparator : public ComparatorBase {
	public:
		virtual ~DefaultComparator() {}

		template<typename T>
		int operator()(const T& a, const T& b) const {
			return a == b ? 0 : a < b ? -1 : 1;
		}

	};

}

/* SkipList */
namespace skiplist {

	template<
		typename _Key,
		typename _Value, 
		size_t _MAX_LEVEL = MAX_LEVEL,
		typename _Comp = DefaultComparator
	>
	class SkipList {

		using KeyType = _Key;
		using ValueType = _Value;
		using CompType = _Comp;

	private:
		struct Node {
			KeyType key;
			ValueType value;
			struct Level {
				Node* next;
			} *level;

			Node(
				int level,
				KeyType key = KeyType(),
				ValueType value = ValueType()
			) :
				key(key),
				value(value) {
				this->level = new Level[level];
				memset(this->level, 0, level * sizeof(Level));
			}

			~Node() {
				if (level) {
					delete[] level;
					level = nullptr;
				}
			}
		};

		int maxLevel_;
		Node* head_;
		CompType comparator_;

	public:
		SkipList() {
			maxLevel_ = 0;
			head_ = createNode(
				_MAX_LEVEL, 
				(std::numeric_limits<KeyType>::min)(), 
				(std::numeric_limits<ValueType>::min)()
			);
		}

		~SkipList() {
			clear();
			delete head_;
			head_ = nullptr;
		}

		int insert(const KeyType& key, const ValueType& value) noexcept {
			int level = randomLevel(); // Get level
			maxLevel_ = maxLevel_ < level ? level : maxLevel_; // Reset max level
			Node* p = head_;
			Node* node = createNode(level, key, value);
			for (int i = maxLevel_ - 1; i >= 0; --i) {
				/* Find prev node of current level */
				while (p->level[i].next && 
					(comparator_(p->level[i].next->key, key) < 0 || 
						(comparator_(p->level[i].next->key, key) == 0 && 
							comparator_(p->level[i].next->value, value) < 0))) {
					p = p->level[i].next;
				}
				/* if current level is less than the node level, then insert it */
				if (i < level) {
					node->level[i].next = p->level[i].next;
					p->level[i].next = node;
				}
			}
			return 0;
		}

		int update(const KeyType& key, const ValueType& value) noexcept {
			Node* node = searchNode(key);
			if (node) {
				node->value = value;
				return 0;
			}
			return 1;
		}

		ValueType remove(const KeyType& key) noexcept {
			Node* p = head_;
			Node* delNode = nullptr;
			ValueType value = -1;
			for (int i = maxLevel_ - 1; i >= 0; --i) {
				/* Find prev node of current level */
				while (p->level[i].next && comparator_(p->level[i].next->key, key) < 0) {
					p = p->level[i].next;
				}
				/* If find delete node, then reset next node */
				if (p->level[i].next && comparator_(p->level[i].next->key, key) == 0) {
					/* actually when level is 0, the delNode is the real node will be delete */
					delNode = p->level[i].next; 
					value = p->level[i].next->value;
					p->level[i].next = p->level[i].next->level[i].next;
				}
			}
			delete delNode;
			delNode = nullptr;
			return value;
		}

		ValueType search(const KeyType& key) noexcept {
			Node* node = searchNode(key);
			return node ? node->value : (std::numeric_limits<ValueType>::min)();
		}

		void clear() {
			while (head_->level[0].next) {
				Node* delNode = head_->level[0].next;
				for (int i = 0; i < maxLevel_; ++i) {
					if (head_->level[i].next) {
						head_->level[i].next = head_->level[i].next->level[i].next;
					}
				}
				delete delNode;
				delNode = nullptr;
			}
		}

	private:
		inline Node* searchNode(const KeyType& key) const noexcept {
			Node* p = head_;
			for (int i = maxLevel_ - 1; i >= 0; --i) {
				while (p->level[i].next && comparator_(p->level[i].next->key, key) <= 0) {
					p = p->level[i].next;
				}
			}
			return p->key == key ? p : nullptr;
		}

		inline int randomLevel(const int& level = MAX_LEVEL) const noexcept {
			static std::mt19937_64 mt;
			long long rd = mt();
			for (int i = 0; i < level; ++i) {
				if ((rd & (1 << i)) == 0) {
					return i + 1;
				}
			}
			return level;
		}

		inline Node* createNode(int level, const KeyType& key, const ValueType& value) const noexcept {
			/*Node* node = static_cast<Node*>(malloc(sizeof(*node) + level*sizeof(Node::Level)));
			if (node) {
				node->key = key;
				node->value = value;
				memset(node->level, 0, level * sizeof(struct Node::Level));
			}
			return node;*/
			return new Node(level, key, value);
		}

	};

}