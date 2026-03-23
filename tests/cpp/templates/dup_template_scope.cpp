// Guards the overload-list cloning in dup_template_scope()/clone_bindlist().

template<class T>
struct Box {
    T value;
};

Box<int> box;
