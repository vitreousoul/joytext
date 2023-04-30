u64 ClampU64(u64 Min, u64 Value, u64 Max);
u64 MinU64(u64 ValueA, u64 ValueB);
u64 MaxU64(u64 ValueA, u64 ValueB);

u64 MinU64(u64 ValueA, u64 ValueB)
{
    return ValueA < ValueB ? ValueA : ValueB;
}

u64 MaxU64(u64 ValueA, u64 ValueB)
{
    return ValueA > ValueB ? ValueA : ValueB;
}

u64 ClampU64(u64 Min, u64 Value, u64 Max)
{
    return MinU64(Max, MaxU64(Min, Value));
}
