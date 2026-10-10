// 1 つの向きの長さを項目(Box の項目・Grid の行や列)に分ける(内部。docs/adr/0017)。
#ifndef TOKOTACHI_SRC_LAYOUT_DISTRIBUTE_HPP
#define TOKOTACHI_SRC_LAYOUT_DISTRIBUTE_HPP

#include <algorithm>
#include <vector>

namespace tokotachi
{
namespace detail
{

// sizes は望む大きさで渡す。available(間隔を除いた長さ)に合わせて書き換える。
// 余るときは、伸びる割合(stretch)に応じて分ける(最大を超えた分は他の伸びる項目へ回す)。どの項目も伸びなければ余りは残す。
// 足りないときは、各項目を最小の大きさまで、望む大きさとの差に応じて縮める(最小の合計でも足りなければ、はみ出す)。
inline void distribute(std::vector<float>& sizes, const std::vector<float>& minimums, const std::vector<float>& maximums,
                       const std::vector<int>& stretches, float available)
{
    const std::size_t n = sizes.size();
    float total = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        total += sizes[i];
    }
    float extra = available - total;
    if (extra > 0)
    {
        std::vector<bool> full(n, false);
        // 最大で止まった項目の分を、他の伸びる項目へ回しながら分ける(回すたびに止まる項目が 1 つ以上増えるので、n 回で終わる)。
        for (std::size_t round = 0; round < n && extra > 0.001f; ++round)
        {
            int weight = 0;
            for (std::size_t i = 0; i < n; ++i)
            {
                if (!full[i] && stretches[i] > 0)
                {
                    weight += stretches[i];
                }
            }
            if (weight == 0)
            {
                break;
            }
            float given = 0;
            for (std::size_t i = 0; i < n; ++i)
            {
                if (full[i] || stretches[i] <= 0)
                {
                    continue;
                }
                const float share = extra * stretches[i] / weight;
                const float room = maximums[i] - sizes[i];
                const float add = std::min(share, room);
                sizes[i] += add;
                given += add;
                if (share >= room)
                {
                    full[i] = true;
                }
            }
            extra -= given;
        }
    }
    else if (extra < 0)
    {
        float shrinkable = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            shrinkable += std::max(0.0f, sizes[i] - minimums[i]);
        }
        if (shrinkable > 0)
        {
            const float ratio = std::min(1.0f, -extra / shrinkable);
            for (std::size_t i = 0; i < n; ++i)
            {
                sizes[i] -= std::max(0.0f, sizes[i] - minimums[i]) * ratio;
            }
        }
    }
}

}  // namespace detail
}  // namespace tokotachi

#endif
