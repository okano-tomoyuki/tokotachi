#include <tokotachi/core/binding.hpp>

#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>

#include "binding_registry.hpp"

namespace tokotachi
{
namespace detail
{

namespace
{

// 結びつきは、両端のプロパティのアドレスと、両端の所有者から引けるようにする。
// メインスレッドだけで扱うので排他はしない。
struct Registry
{
    std::set<BindingBase*> alive;
    std::unordered_map<const void*, std::vector<BindingBase*> > by_property;
    std::unordered_map<Object*, std::vector<BindingBase*> > by_owner;
};

// 静的なオブジェクトの破棄の順番によらず使えるよう、わざと解放しない。
Registry& registry()
{
    static Registry* r = new Registry;
    return *r;
}

template <typename K>
void erase_from(std::unordered_map<K, std::vector<BindingBase*> >& map, K key, BindingBase* b)
{
    typename std::unordered_map<K, std::vector<BindingBase*> >::iterator it = map.find(key);
    if (it == map.end())
    {
        return;
    }
    std::vector<BindingBase*>& v = it->second;
    v.erase(std::remove(v.begin(), v.end(), b), v.end());
    if (v.empty())
    {
        map.erase(it);
    }
}

void remove(BindingBase* b)
{
    Registry& r = registry();
    if (!r.alive.erase(b))
    {
        return;
    }
    erase_from(r.by_property, b->a.property, b);
    erase_from(r.by_property, b->b.property, b);
    erase_from(r.by_owner, b->a.owner, b);
    if (b->b.owner != b->a.owner)
    {
        erase_from(r.by_owner, b->b.owner, b);
    }
    delete b;
}

bool same_pair(const BindingBase* x, const void* a, const void* b)
{
    return (x->a.property == a && x->b.property == b) || (x->a.property == b && x->b.property == a);
}

}  // namespace

bool add_binding(BindingBase* b)
{
    Registry& r = registry();
    if (b->a.property == b->b.property)
    {
        delete b;
        return false;
    }
    std::unordered_map<const void*, std::vector<BindingBase*> >::iterator it = r.by_property.find(b->a.property);
    if (it != r.by_property.end())
    {
        for (std::size_t i = 0; i < it->second.size(); ++i)
        {
            if (same_pair(it->second[i], b->a.property, b->b.property))
            {
                delete b;
                return false;
            }
        }
    }
    r.alive.insert(b);
    r.by_property[b->a.property].push_back(b);
    r.by_property[b->b.property].push_back(b);
    r.by_owner[b->a.owner].push_back(b);
    if (b->b.owner != b->a.owner)
    {
        r.by_owner[b->b.owner].push_back(b);
    }
    // 結んだ時点で a を b の値にそろえる(a の書き込みの関数が a の変更を知らせ、a の他の結びつきへも伝わる)。
    b->active = true;
    b->propagate(false);
    if (r.alive.count(b))
    {
        b->active = false;
    }
    return true;
}

void remove_bindings(Object*, const void* property)
{
    Registry& r = registry();
    std::unordered_map<const void*, std::vector<BindingBase*> >::iterator it = r.by_property.find(property);
    if (it == r.by_property.end())
    {
        return;
    }
    const std::vector<BindingBase*> list(it->second);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        remove(list[i]);
    }
}

void remove_binding_pair(const void* a, const void* b)
{
    Registry& r = registry();
    std::unordered_map<const void*, std::vector<BindingBase*> >::iterator it = r.by_property.find(a);
    if (it == r.by_property.end())
    {
        return;
    }
    const std::vector<BindingBase*> list(it->second);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if (same_pair(list[i], a, b))
        {
            remove(list[i]);
        }
    }
}

void remove_bindings_of(Object* owner)
{
    Registry& r = registry();
    std::unordered_map<Object*, std::vector<BindingBase*> >::iterator it = r.by_owner.find(owner);
    if (it == r.by_owner.end())
    {
        return;
    }
    const std::vector<BindingBase*> list(it->second);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        remove(list[i]);
    }
}

std::size_t binding_count() { return registry().alive.size(); }

void property_changed(Object*, const void* property)
{
    Registry& r = registry();
    if (r.alive.empty())
    {
        return;
    }
    std::unordered_map<const void*, std::vector<BindingBase*> >::iterator it = r.by_property.find(property);
    if (it == r.by_property.end())
    {
        return;
    }
    // 伝えている間に結びつきが外れる・オブジェクトが破棄されることがあるので、写してから、生きているものだけ扱う。
    const std::vector<BindingBase*> list(it->second);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        BindingBase* b = list[i];
        if (!r.alive.count(b) || b->active)
        {
            continue;
        }
        // 伝えている間は active にしておき、相手の書き込みの関数が知らせた変更がこの結びつきを戻ってこないようにする(循環の防止)。
        b->active = true;
        b->propagate(b->a.property == property);
        if (r.alive.count(b))
        {
            b->active = false;
        }
    }
}

}  // namespace detail
}  // namespace tokotachi
