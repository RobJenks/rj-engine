
// Return the median of three inputs.  Branchless assuming GPU instr for min/max
float median(float a, float b, float c)
{
	return max(min(a, b), min(max(a, b), c));
}