
class CtorDtorVerifier : private boost::noncopyable {
public:
	CtorDtorVerifier(int val)
		: mVal(val) {
		if (!mConstructedAddresses.insert(this).second) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(int) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier(size_t val)
		: mVal(static_cast<int>(val)) {
		if (!mConstructedAddresses.insert(this).second) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(size_t) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier()
		: mVal(-1) {
		if (!mConstructedAddresses.insert(this).second) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor() " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier(const CtorDtorVerifier& o)
		: mVal(o.mVal) {
		if (!mConstructedAddresses.insert(this).second) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(const CtorDtorVerifier& o) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier& operator=(CtorDtorVerifier&& o) {
		if (1 != mConstructedAddresses.count(this)) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (1 != mConstructedAddresses.count(&o)) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		mVal = std::move(o.mVal);
		return *this;
	}

	CtorDtorVerifier& operator=(const CtorDtorVerifier& o) {
		if (1 != mConstructedAddresses.count(this)) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (1 != mConstructedAddresses.count(&o)) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		mVal = o.mVal;
		return *this;
	}

	~CtorDtorVerifier() {
		if (1 != mConstructedAddresses.erase(this)) {
			std::cerr << "ERROR" << std::endl;
			EXPECT_TRUE(false);
		}
		if (mDoPrintDebugInfo) {
			std::cout << this << " dtor " << mConstructedAddresses.size() << std::endl;
		}
	}

	bool eq(const CtorDtorVerifier& o) const {
		return mVal == o.mVal;
	}

	int val() const {
		return mVal;
	}

	static size_t mapSize() {
		return mConstructedAddresses.size();
	}

	static void printMap() {
		std::cout << "data in map:" << std::endl;
		for (boost::unordered_set<CtorDtorVerifier const*>::const_iterator it = mConstructedAddresses.begin(), end = mConstructedAddresses.end();
			 it != end; ++it) {
			std::cout << "\t" << *it << std::endl;
		}
	}

	static bool contains(CtorDtorVerifier const* ptr) {
		return 1 == mConstructedAddresses.count(ptr);
	}

private:
	int mVal;
	static boost::unordered_set<CtorDtorVerifier const*> mConstructedAddresses;
	static bool mDoPrintDebugInfo;
};

boost::unordered_set<CtorDtorVerifier const*> CtorDtorVerifier::mConstructedAddresses;
bool CtorDtorVerifier::mDoPrintDebugInfo = false;

bool operator==(const CtorDtorVerifier& a, const CtorDtorVerifier& b) {
	return a.eq(b);
}

namespace util {
namespace RobinHood {

template <>
struct FastHash<CtorDtorVerifier> {
	std::size_t operator()(const CtorDtorVerifier& t) const {
		// hash is bad on purpose
		const size_t bitmaskWithoutLastBits = ~(size_t)5;
		return static_cast<size_t>(t.val()) & bitmaskWithoutLastBits;
	}
};

} // namespace RobinHood
} // namespace util

// Dummy hash for testing collisions
template <class T>
struct DummyHash {
	std::size_t operator()(const T&) const {
		return 123;
	}
};

template <class Map>
void testAssignmentCombinations() {
	{
		Map a;
		Map b;
		b = a;
	}
	{
		Map a;
		Map const& aConst = a;
		Map b;
		a[123] = 321;
		b = a;

		REQUIRE(a.find(123)->second == 321);
		REQUIRE(aConst.find(123)->second == 321);

		REQUIRE(b.find(123)->second == 321);
		a[123] = 111;
		REQUIRE(a.find(123)->second == 111);
		REQUIRE(aConst.find(123)->second == 111);
		REQUIRE(b.find(123)->second == 321);
		b[123] = 222;
		REQUIRE(a.find(123)->second == 111);
		REQUIRE(aConst.find(123)->second == 111);
		REQUIRE(b.find(123)->second == 222);
	}

	{
		Map a;
		Map b;
		a[123] = 321;
		a.clear();
		b = a;

		REQUIRE(a.size() == 0);
		REQUIRE(b.size() == 0);
	}

	{
		Map a;
		Map b;
		b[123] = 321;
		b = a;

		REQUIRE(a.size() == 0);
		REQUIRE(b.size() == 0);
	}
	{
		Map a;
		Map b;
		b[123] = 321;
		b.clear();
		b = a;

		REQUIRE(a.size() == 0);
		REQUIRE(b.size() == 0);
	}
	{
		Map a;
		a[1] = 2;
		Map b;
		b[3] = 4;
		b = a;

		REQUIRE(a.size() == 1);
		REQUIRE(b.size() == 1);
		REQUIRE(b.find(1)->second == 2);
		a[1] = 123;
		REQUIRE(a.size() == 1);
		REQUIRE(b.size() == 1);
		REQUIRE(b.find(1)->second == 2);
	}
	{
		Map a;
		a[1] = 2;
		a.clear();
		Map b;
		b[3] = 4;
		b = a;
	}
	{
		Map a;
		a[1] = 2;
		Map b;
		b[3] = 4;
		b.clear();
		b = a;
	}
	{
		Map a;
		a[1] = 2;
		a.clear();
		Map b;
		b[3] = 4;
		b.clear();
		b = a;
	}
}

template <class M>
void taic() {
	{
		M m;
		fill(m, 1);
		for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) {
			REQUIRE(CtorDtorVerifier::contains(&it->first));
			REQUIRE(CtorDtorVerifier::contains(&it->second));
		}
		// m.clear();
	}
	if (0 != CtorDtorVerifier::mapSize()) {
		CtorDtorVerifier::printMap();
	}
	REQUIRE(CtorDtorVerifier::mapSize() == 0);
}

TEST(RobinHoodMapTest, testAssignIterateClear) {
	taic<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, util::RobinHood::FastHash<CtorDtorVerifier>,
							  util::RobinHood::EqualTo<CtorDtorVerifier>, false>>();
	taic<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, util::RobinHood::FastHash<CtorDtorVerifier>,
							  util::RobinHood::EqualTo<CtorDtorVerifier>, true>>();
}

TEST(RobinHoodMapTest, testAllAssignCombinations) {
	testAssignmentCombinations<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, util::RobinHood::FastHash<CtorDtorVerifier>,
													util::RobinHood::EqualTo<CtorDtorVerifier>, false>>();
	testAssignmentCombinations<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, util::RobinHood::FastHash<CtorDtorVerifier>,
													util::RobinHood::EqualTo<CtorDtorVerifier>, true>>();
	testAssignmentCombinations<util::RobinHood::Map<int, int, util::RobinHood::FastHash<int>, util::RobinHood::EqualTo<int>, false>>();
	testAssignmentCombinations<util::RobinHood::Map<int, int, util::RobinHood::FastHash<int>, util::RobinHood::EqualTo<int>, true>>();
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);
}

TEST(RobinHoodMapTest, testIterators) {
	for (size_t i = 0; i < 10; ++i) {
		util::RobinHood::Map<int, int> m;
		REQUIRE(m.begin() == m.end());

		REQUIRE(m.end() == m.find(132));

		m[1];
		REQUIRE(m.begin() != m.end());
		REQUIRE(++m.begin() == m.end());
		m.clear();

		REQUIRE(m.begin() == m.end());
	}
}

TEST(RobinHoodMapTest, SLOW_simpleTest) {
	using Map = util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, util::RobinHood::FastHash<CtorDtorVerifier>,
									 util::RobinHood::EqualTo<CtorDtorVerifier>, true>;
	Map rhhs;
	REQUIRE(rhhs.size() == (size_t)0);
	std::pair<Map::iterator, bool> it = rhhs.insert(typename Map::value_type(32145, 123));
	REQUIRE(it.second);
	REQUIRE(it.first->first == 32145);
	REQUIRE(it.first->second == 123);
	REQUIRE(rhhs.size() == (size_t)1);

	const int times = 10000;
	for (int i = 0; i < times; ++i) {
		std::pair<Map::iterator, bool> it = rhhs.insert(typename Map::value_type(i * 4, i));

		REQUIRE(it.second);
		REQUIRE(it.first->first == i * 4);
		REQUIRE(it.first->second == i);

		Map::iterator found = rhhs.find(i * 4);
		REQUIRE(rhhs.end() != found);
		REQUIRE(found->second == i);
		REQUIRE(rhhs.size() == (size_t)(2 + i));
	}

	// check if everything can be found
	for (int i = 0; i < times; ++i) {
		Map::iterator found = rhhs.find(i * 4);
		REQUIRE(rhhs.end() != found);
		REQUIRE(found->second == i);
		REQUIRE(found->first == i * 4);
	}

	// check non-elements
	for (int i = 0; i < times; ++i) {
		Map::iterator found = rhhs.find((i + times) * 4);
		REQUIRE(rhhs.end() == found);
	}

	// random test against std::unordered_map
	rhhs.clear();
	boost::unordered_map<int, int> uo;

	boost::mt19937 gen;
	gen.seed(123);
	boost::random::uniform_int_distribution<> dist(0, times / 4);

	for (int i = 0; i < times; ++i) {
		int r = dist(gen);
		std::pair<Map::iterator, bool> rhh_it = rhhs.insert(typename Map::value_type(r, r * 2));
		std::pair<boost::unordered_map<int, int>::iterator, bool> uo_it = uo.insert(std::make_pair(r, r * 2));
		REQUIRE(rhh_it.second == uo_it.second);
		REQUIRE(rhh_it.first->first == uo_it.first->first);
		REQUIRE(rhh_it.first->second == uo_it.first->second);
		REQUIRE(rhhs.size() == uo.size());

		r = dist(gen);
		Map::iterator rhhsIt = rhhs.find(r);
		boost::unordered_map<int, int>::iterator uoIt = uo.find(r);
		REQUIRE(rhhs.end() == rhhsIt == uo.end() == uoIt);
		if (rhhs.end() != rhhsIt) {
			REQUIRE(rhhsIt->first == uoIt->first);
			REQUIRE(rhhsIt->second == uoIt->second);
		}
	}

	uo.clear();
	rhhs.clear();
	for (int i = 0; i < times; ++i) {
		const int r = dist(gen);
		rhhs[r] = r * 2;
		uo[r] = r * 2;
		REQUIRE(rhhs.find(r)->second == uo.find(r)->second);
		REQUIRE(rhhs.size() == uo.size());
	}

	std::size_t numChecks = 0;
	for (Map::const_iterator it = rhhs.begin(); it != rhhs.end(); ++it) {
		REQUIRE(uo.end() != uo.find(it->first.val()));
		++numChecks;
	}
	REQUIRE(rhhs.size() == numChecks);

	numChecks = 0;
	const Map& constRhhs = rhhs;
	BOOST_FOREACH (const Map::value_type& vt, constRhhs) {
		REQUIRE(uo.end() != uo.find(vt.first.val()));
		++numChecks;
	}
	REQUIRE(rhhs.size() == numChecks);
}

struct BigObject {
	std::string str;
	std::vector<int> vec;
	std::shared_ptr<int> ptr;
	std::list<int> list;
};

// provide a dummy hash
namespace std {
template <>
struct hash<BigObject> {
	size_t operator()(const BigObject& md) const {
		return 0;
	}
};

} // namespace std

template <class T>
void benchmarkCtor(size_t fixedNumIters, const std::string& typeName) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);
	std::size_t optPrevention = 0;

	if (mb.isAutoTimed()) {
		std::string msg = std::string("ctor boost::unordered_map<int, ") + typeName + std::string(">");
		const char* title = msg.c_str();
		while (mb(title, optPrevention)) {
			boost::unordered_map<int, T, util::RobinHood::FastHash<int>> m;
			util::MicroBenchmark::doNotOptimize(m);
		}
	}

	optPrevention = 0;
	std::string msg = std::string("ctor util::RobinHood::Map<int, ") + typeName + std::string(">");
	const char* title = msg.c_str();
	while (mb(title)) {
		util::RobinHood::Map<int, T, util::RobinHood::FastHash<int>> m;
		util::MicroBenchmark::doNotOptimize(m);
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCtorInt) {
	benchmarkCtor<int>(20 * 1000 * 1000, "int");
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCtorBigObject) {
	benchmarkCtor<BigObject>(20 * 1000 * 1000, "BigObject");
}

template <class M>
void benchCopy(util::MicroBenchmark& mb, const uint32_t param, const char* title) {
	// fill map with random data
	boost::mt19937 gen(123);
	M map;
	for (uint32_t i = 0; i < param; ++i) {
		map[gen() % param];
	}

	std::size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		M mapCpy = map;
		optPrevention += mapCpy.size();
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCopyInt) {
	uint32_t numElements = 5000;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(100 * 1000);

	if (mb.isAutoTimed()) {
		benchCopy<boost::unordered_map<int, int, util::RobinHood::FastHash<int>>>(mb, numElements, "copy boost::unordered_map<int, int>");
	}
	benchCopy<util::RobinHood::Map<int, int>>(mb, numElements, "copy util::RobinHood<int, int>");
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCopyBigObject) {
	uint32_t numElements = 5000;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(500);
	if (mb.isAutoTimed()) {
		benchCopy<boost::unordered_map<int, BigObject, util::RobinHood::FastHash<int>>>(mb, numElements, "copy boost::unordered_map<int, BigObject>");
	}
	benchCopy<util::RobinHood::Map<int, BigObject>>(mb, numElements, "copy util::RobinHood<int, BigObject>");
}

template <class T>
void benchmarkCtorAndAddElements(size_t fixedNumIters, size_t numInserts, const std::string& typeName) {
	if (fixedNumIters == 0) {
		fixedNumIters = 1;
	}
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);
	mb.unitScaling("insert", numInserts);
	size_t pos = 0;

	if (mb.isAutoTimed()) {
		std::string msg = std::string("boost::unordered_map<int, ") + typeName + std::string(">");
		const char* title = msg.c_str();
		size_t noOpt = 0;
		while (mb(title, noOpt)) {
			boost::unordered_map<size_t, T, util::RobinHood::FastHash<size_t>> m;
			for (size_t i = 0; i < numInserts; ++i) {
				m[pos++];
			}
			noOpt += m.size();
		}
	}

	pos = 0;
	std::stringstream ss;
	ss << "util::RobinHood::Map<int, " << typeName << ">, " << numInserts << " elements";
	std::string str = ss.str();
	const char* title = str.c_str();

	size_t noOpt = 0;
	while (mb(title, noOpt)) {
		util::RobinHood::Map<size_t, T> m;
		for (size_t i = 0; i < numInserts; ++i) {
			m[pos++];
		}
		noOpt += m.size();
	}
}

#define PRINT_SIZEOF(x, A, B) std::cout << sizeof(x<A, B>) << " bytes for " #x "<" #A ", " #B ">" << std::endl

TEST(RobinHoodMapTest, testSizeof) {
	util::RobinHood::Map<int, int> a;
	util::RobinHood::Map<BigObject, BigObject> b;

	PRINT_SIZEOF(util::RobinHood::Map, int, int);
	PRINT_SIZEOF(std::map, int, int);
	PRINT_SIZEOF(boost::unordered_map, int, int);
	PRINT_SIZEOF(std::unordered_map, int, int);
	std::cout << std::endl;

	PRINT_SIZEOF(util::RobinHood::Map, int, BigObject);
	PRINT_SIZEOF(std::map, int, BigObject);
	PRINT_SIZEOF(boost::unordered_map, int, BigObject);
	PRINT_SIZEOF(std::unordered_map, int, BigObject);
	std::cout << std::endl;

	PRINT_SIZEOF(util::RobinHood::Map, BigObject, BigObject);
	PRINT_SIZEOF(std::map, BigObject, BigObject);
	PRINT_SIZEOF(boost::unordered_map, BigObject, BigObject);
	PRINT_SIZEOF(std::unordered_map, BigObject, BigObject);
	std::cout << std::endl;

	PRINT_SIZEOF(util::RobinHood::Map, BigObject, int);
	PRINT_SIZEOF(std::map, BigObject, int);
	PRINT_SIZEOF(boost::unordered_map, BigObject, int);
	PRINT_SIZEOF(std::unordered_map, BigObject, int);
}

TEST(RobinHoodMapTest, testCtorAndSingleElementAdded) {
	boost::unordered_map<int, BigObject> uo;
	uo[123];

	util::RobinHood::Map<int, BigObject> rh;
	rh[123];
}

class RobinHoodMapSize10Params : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_CASE_P(RobinHoodInstance, RobinHoodMapSize10Params, ::testing::Values(1, 10, 100, 1000, 10000, 100000));

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkCtorAndSingleElementAddedBigObject) {
	size_t size = static_cast<size_t>(GetParam());
	benchmarkCtorAndAddElements<BigObject>(800 * 1000 / size, size, "BigObject");
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkCtorAndSingleElementAddedInt) {
	size_t size = static_cast<size_t>(GetParam());
	benchmarkCtorAndAddElements<BigObject>(1000 * 1000 / size, size, "int");
}

template <class M>
void fill(M& map, size_t num, bool isExisting = true) {
	if (isExisting) {
		for (size_t i = 0; i < num; ++i) {
			map[static_cast<typename M::key_type>(i)];
		}
	} else {
		for (size_t i = 0; i < num; ++i) {
			map[static_cast<typename M::key_type>(i + num)];
		}
	}
}

inline uint32_t limit(uint32_t r, uint32_t endExclusive) {
	uint64_t x = r;
	x *= endExclusive;
	return static_cast<uint32_t>(x >> 32);
}

void benchFind(bool isExisting, uint32_t numElements, size_t iters) {
	boost::mt19937 gen(123);

	util::MicroBenchmark mb;

	mb.fixedIterationsPerMeasurement(iters);

	if (mb.isAutoTimed()) {
		boost::unordered_map<int, int, util::RobinHood::FastHash<size_t>> uo;
		fill(uo, numElements, isExisting);
		std::size_t optPrevention = 0;
		while (mb("boost::unordered_map<size_t, size_t>::find", optPrevention)) {
			if (uo.end() != uo.find(limit(gen(), numElements))) {
				optPrevention++;
			}
		}
	}

	util::RobinHood::Map<int, int> rh;
	fill(rh, numElements, isExisting);
	std::size_t optPrevention = 0;
	while (mb("util::RobinHood::Map<size_t, size_t>::find", optPrevention)) {
		if (rh.end() != rh.find(limit(gen(), numElements))) {
			optPrevention++;
		}
	}
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkFindExisting) {
	uint32_t size = static_cast<uint32_t>(GetParam());
	benchFind(true, size, 10 * 1000 * 1000);
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkFindNonExisting) {
	uint32_t size = static_cast<uint32_t>(GetParam());
	benchFind(false, size, 10 * 1000 * 1000);
}

template <class Map>
void benchIterate(util::MicroBenchmark& mb, size_t numElements, const char* name) {
	boost::mt19937 gen;
	gen.seed(1234);

	// fill map
	Map m;
	for (size_t i = 0; i < numElements; ++i) {
		m[gen()] = gen();
	}

	size_t optPrevention = 0;
	while (mb(name, optPrevention)) {
		for (typename Map::const_iterator it = m.begin(), end = m.end(); it != end; ++it) {
			optPrevention += it->second;
		}
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkIteration) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(2000);

	if (mb.isAutoTimed()) {
		benchIterate<boost::unordered_map<size_t, size_t>>(mb, 10000, "iterating boost::unordered_map");
	}

	benchIterate<util::RobinHood::Map<size_t, size_t>>(mb, 10000, "iterating util::RobinHood::Map");
}

template <class Map>
void benchIterateAlwaysEnd(util::MicroBenchmark& mb, size_t numElements, const char* name) {
	boost::mt19937 gen;
	gen.seed(1234);

	// fill map
	Map m;
	for (size_t i = 0; i < numElements; ++i) {
		m[gen()] = gen();
	}

	size_t optPrevention = 0;
	while (mb(name, optPrevention)) {
		for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it) {
			optPrevention += it->second;
		}
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkIterationAlwaysEnd) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(2000);

	if (mb.isAutoTimed()) {
		benchIterateAlwaysEnd<boost::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t>>>(mb, 10000, "iterating boost::unordered_map");
	}
	benchIterateAlwaysEnd<util::RobinHood::Map<size_t, size_t>>(mb, 10000, "iterating util::RobinHood::Map");
}

class RobinHoodMapTestRandomTest : public testing::TestWithParam<int> {};

template <class M>
void randomMapInsertDelete(size_t maxElements, util::MicroBenchmark& mb, const char* name) {
	boost::mt19937 gen;
	gen.seed(321);

	M m;
	while (mb(name)) {
		m[gen() % maxElements];
		m.erase(gen() % maxElements);
	}
	mb.noOpt(m.size());
}

template <class T>
void benchRandomInsertDelete(size_t fixedNumIters, const char* title, int s) {
	util::MicroBenchmark mb;

	std::stringstream ss;
	ss << ", " << s << " entries max for " << title << " (" << sizeof(T) << " bytes)";
	mb.clearBaseline();
	mb.fixedIterationsPerMeasurement(fixedNumIters);

	if (mb.isAutoTimed()) {
		randomMapInsertDelete<boost::unordered_map<int, T, util::RobinHood::FastHash<int>>>(s, mb, ("boost::unordered_map" + ss.str()).c_str());
		// randomMapInsertDelete<std::map<int, T> >(s, mb, ("std::map" + ss.str()).c_str());
		// randomMapInsertDelete<boost::container::flat_map<int, T> >(s, mb, ("boost::container::flat_map" + ss.str()).c_str());
	}

	randomMapInsertDelete<util::RobinHood::Map<int, T>>(s, mb, ("util::RobinHood::Map" + ss.str()).c_str());
}

class RobinHoodMapSizeParams : public ::testing::TestWithParam<int> {};

TEST_P(RobinHoodMapSizeParams, PERFORMANCE_SLOW_benchmarkRandomInsertDeleteInt) {
	benchRandomInsertDelete<int>(2 * 1000 * 1000, "int", GetParam());
}

TEST_P(RobinHoodMapSizeParams, PERFORMANCE_SLOW_benchmarkRandomInsertDeleteBigObject) {
	benchRandomInsertDelete<BigObject>(1000 * 1000, "BigObject", GetParam());
}

INSTANTIATE_TEST_CASE_P(RobinHoodInstance, RobinHoodMapSizeParams,
						::testing::Values(0x1F * 100 / 50, 0x1FF * 100 / 70, 0x1FFFF * 100 / 51, 0x1FFFF * 100 / 70, 0x1FFFF * 100 / 80,
										  0x1FFFF * 100 / 90, 0x1FFFF * 100 / 95));

TEST_P(RobinHoodMapTestRandomTest, SLOW_verifyRandomInsertDelete) {
	util::MicroBenchmark mb(1, 0.3);
	randomMapInsertDelete<util::RobinHood::Map<int, CtorDtorVerifier>>(GetParam(), mb, "CtorDtorVerifier");
	REQUIRE(0 == CtorDtorVerifier::mapSize());
}

template <class Map>
size_t check(const size_t num) {
	boost::mt19937 gen;
	gen.seed(static_cast<uint32_t>(num));

	Map map;
	for (size_t i = 0; i < num; ++i) {
		map[gen() % 5000] = typename Map::mapped_type(i);
		map.erase(gen() % 5000);
	}

	// generateds a somewhat reasonable representation of the map, that's independent of the ordering.
	std::size_t hash = map.size();
	for (typename Map::const_iterator it = map.begin(), end = map.end(); it != end; ++it) {
		hash *= (std::max)(static_cast<size_t>(1), 1779033703 + it->first * 123 + it->second);
	}
	return hash;
}

// Performs lots of assign and erase operations, then checks if the maps have the same content.
TEST(RobinHoodMapTest, SLOW_testRandomInserteDeleteSameAsBoostMap) {
	for (size_t i = 0; i < 500; ++i) {
		size_t a = check<boost::unordered_map<size_t, size_t>>(i);
		size_t b = check<util::RobinHood::Map<size_t, size_t, util::RobinHood::FastHash<size_t>, util::RobinHood::EqualTo<size_t>, false>>(i);
		size_t c = check<util::RobinHood::Map<size_t, size_t, util::RobinHood::FastHash<size_t>, util::RobinHood::EqualTo<size_t>, true>>(i);
		REQUIRE(b == a);
		REQUIRE(c == a);
	}
}

static size_t sCountCtor = 0;
static size_t sCountDefaultCtor = 0;
static size_t sCountCopyCtor = 0;
static size_t sCountDtor = 0;
static size_t sCountEquals = 0;
static size_t sCountLess = 0;
static size_t sCountAssign = 0;
static size_t sCountSwaps = 0;
static size_t sCountGet = 0;
static size_t sCountConstGet = 0;
static size_t sCountHash = 0;
static size_t sCountMoveCtor = 0;
static size_t sCountMoveAssign = 0;

void resetStaticCounts() {
	sCountCtor = 0;
	sCountDefaultCtor = 0;
	sCountCopyCtor = 0;
	sCountDtor = 0;
	sCountEquals = 0;
	sCountLess = 0;
	sCountAssign = 0;
	sCountSwaps = 0;
	sCountGet = 0;
	sCountConstGet = 0;
	sCountHash = 0;
	sCountMoveCtor = 0;
	sCountMoveAssign = 0;
}

void printStaticHeader() {
	printf("    ctor  defctor  cpyctor     dtor   assign     swaps      get  cnstget     hash   equals     less   ctormv assignmv |  total\n");
}
void printStaticCounts(const char* title) {
	size_t total = sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountDtor + sCountEquals + sCountLess + sCountAssign + sCountSwaps + sCountGet +
				   sCountConstGet + sCountHash + sCountMoveCtor + sCountMoveAssign;

	printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", sCountCtor, sCountDefaultCtor, sCountCopyCtor, sCountDtor, sCountAssign,
		   sCountSwaps, sCountGet, sCountConstGet, sCountHash, sCountEquals, sCountLess, sCountMoveCtor, sCountMoveAssign, total, title);
}

template <class T>
class Counter : private boost::noncopyable {
public:
	Counter(const T& data)
		: mData(data) {
		++sCountCtor;
	}

	Counter()
		: mData() {
		++sCountDefaultCtor;
	}

	Counter(const Counter& o)
		: mData(o.mData) {
		++sCountCopyCtor;
	}

	Counter(Counter&& o)
		: mData(std::move(o.mData)) {
		++sCountMoveCtor;
	}

	~Counter() {
		++sCountDtor;
	}

	bool operator==(const Counter& o) const {
		++sCountEquals;
		return mData == o.mData;
	}

	bool operator<(const Counter& o) const {
		++sCountLess;
		return mData < o.mData;
	}

	Counter& operator=(const Counter& o) {
		++sCountAssign;
		mData = o.mData;
		return *this;
	}

	Counter& operator=(Counter&& o) {
		++sCountMoveAssign;
		mData = std::move(o.mData);
		return *this;
	}

	const T& get() const {
		++sCountConstGet;
		return mData;
	}

	T& get() {
		++sCountGet;
		return mData;
	}

	void swap(Counter& other) {
		++sCountSwaps;
		using std::swap;
		swap(mData, other.mData);
	}

private:
	T mData;
};

template <class T>
void swap(Counter<T>& a, Counter<T>& b) {
	a.swap(b);
}

template <class T>
std::size_t hash_value(const Counter<T>& c) {
	++sCountHash;
	return util::RobinHood::FastHash<T>()(c.get());
}

namespace std {

template <class T>
class hash<Counter<T>> {
public:
	size_t operator()(const Counter<T>& c) const {
		++sCountHash;
		return hash<T>()(c.get());
	}
};

} // namespace std

template <class Map>
void randomInsertDelete(const size_t numIters, size_t maxElement) {
	boost::mt19937 gen;
	gen.seed(static_cast<uint32_t>(12332));

	Map map;
	for (size_t i = 0; i < numIters; ++i) {
		map[gen() % maxElement] = i;
		map.erase(gen() % maxElement);
	}

	// non-move call
	typename Map::key_type key = 321;
	for (size_t i = 0; i < numIters; ++i) {
		map[key] = i;
	}
}

template <class T>
struct BigCounter : public Counter<T> {
	BigCounter(const T& data)
		: Counter<T>(data)
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	BigCounter()
		: Counter<T>()
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	BigCounter(const BigCounter& o)
		: Counter<T>(o)
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	double a;
	double b;
	double c;
	double d;
};

// Tests if all objects are properly constructed and destructed
TEST(RobinHoodMapTest, testCountCtorAndDtor) {
	printStaticHeader();

	size_t numIters = 100000;
	size_t maxElements = 100000;

	resetStaticCounts();
	{ randomInsertDelete<util::RobinHood::DirectMap<Counter<size_t>, Counter<size_t>>>(numIters, maxElements); }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts("util::RobinHood::DirectMap<Counter, Counter>");

	resetStaticCounts();
	{ randomInsertDelete<util::RobinHood::IndirectMap<Counter<size_t>, Counter<size_t>>>(numIters, maxElements); }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts("util::RobinHood::IndirectMap<Counter, Counter>");

	resetStaticCounts();
	{ randomInsertDelete<boost::unordered_map<Counter<size_t>, Counter<size_t>>>(numIters, maxElements); }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts("boost::unordered_map");

	resetStaticCounts();
	{ randomInsertDelete<std::unordered_map<Counter<size_t>, Counter<size_t>>>(numIters, maxElements); }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts("std::unordered_map");

	resetStaticCounts();
	{ randomInsertDelete<std::map<Counter<size_t>, Counter<size_t>>>(numIters, maxElements); }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts("std::map");
}

TEST(RobinHoodMapTest, testCountCtorAndDtorFill) {
	printStaticHeader();

	resetStaticCounts();
	{
		util::RobinHood::DirectMap<Counter<size_t>, Counter<size_t>> map;
		fill(map, 100000);
	}
	printStaticCounts("util::RobinHood::DirectMap<Counter, Counter>");
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);

	resetStaticCounts();
	{
		util::RobinHood::IndirectMap<Counter<size_t>, Counter<size_t>> map;
		fill(map, 100000);
	}
	printStaticCounts("util::RobinHood::IndirectMap<Counter, Counter>");
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);

	resetStaticCounts();
	{
		boost::unordered_map<Counter<size_t>, Counter<size_t>> map;
		fill(map, 100000);
	}
	printStaticCounts("boost::unordered_map");
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);

	resetStaticCounts();
	{
		std::unordered_map<Counter<size_t>, Counter<size_t>> map;
		fill(map, 100000);
	}
	printStaticCounts("std::unordered_map");
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
}

class Node {
public:
	Node(int x) {}
};

// TODO this test does not compile if Node has no default constructor :-(
TEST(RobinHoodMapTest, testNoDefaultCtor) {
	std::pair<size_t, Node> p(123, Node(123));
	{
		boost::unordered_map<size_t, Node> map;
		// util::RobinHood::Map<size_t, Node> map;
		map.insert(p);
	}
}

TEST(RobinHoodMapTest, testUniquePtr) {
	util::RobinHood::Map<int, std::unique_ptr<int>> map;
	map[321].reset(new int(123));
}

TEST(RobinHoodMapTest, testEraseIterator) {
	typedef util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier> M;
	{
		M map;
		for (int i = 0; i < 100; ++i) {
			map[i * 101] = i * 101;
		}

		M::const_iterator it = map.find(20 * 101);
		REQUIRE(map.size() == 100);
		REQUIRE(map.end() != map.find(20 * 101));
		it = map.erase(it);
		REQUIRE(map.size() == 99);
		REQUIRE(map.end() == map.find(20 * 101));

		it = map.begin();
		size_t currentSize = map.size();
		while (it != map.end()) {
			it = map.erase(it);
			currentSize--;
			REQUIRE(map.size() == currentSize);
		}
		REQUIRE(map.size() == (size_t)0);
	}
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);
}

TEST(RobinHoodMapTest, testVector) {
	typedef util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier> Map;
	{
		std::vector<Map> maps;
		for (size_t i = 0; i < 10; ++i) {
			Map m;
			fill(m, 100);
			maps.push_back(m);
		}
	}
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);
}

TEST(RobinHoodMapTest, testMapOfMaps) {
	{
		util::RobinHood::Map<CtorDtorVerifier, util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier>> maps;
		for (size_t i = 0; i < 10; ++i) {
			fill(maps[(int)i], 100);
		}

		util::RobinHood::Map<CtorDtorVerifier, util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier>> maps2;
		maps2 = maps;
	}
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);
}

template <class I>
struct PassThroughHash {
	inline size_t operator()(const I& i) const {
		return static_cast<size_t>(i);
	}
};

void showHash(apr_uint64_t val) {
	PassThroughHash<apr_uint64_t> ph;
	boost::hash<apr_uint64_t> bh;
	util::RobinHood::FastHash<apr_uint64_t> fh;
	printf("h(0x%016" APR_UINT64_T_HEX_FMT "): 0x%016zx 0x%016zx 0x%016zx\n", val, ph(val), bh(val), fh(val));
}

template <class H>
void showHashSamples(const std::string& str) {}

TEST(RobinHoodMapTest, showHashDistribution) {
	std::cout << "                          PassThroughHash    boost::hash     util::RobinHood::FastHash" << std::endl;
	for (apr_uint64_t i = 0; i < 5; ++i) {
		showHash(i);
	}

	for (apr_uint64_t i = 0; i < 5; ++i) {
		showHash(0x000023d700000063ULL + i * 0x100000000ULL);
	}

	for (apr_uint64_t i = 0; i < 5; ++i) {
		showHash(i * 0x1000000000000000ULL);
	}

	for (apr_uint64_t i = 1; i != 0; i *= 2) {
		showHash(i);
	}
}

template <class I>
void benchHash(size_t fixedNumIters, const std::string& titleStr) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);

	if (mb.isAutoTimed()) {
		std::string msg = "PassThroughHash<" + titleStr + ">";
		const char* title = msg.c_str();

		PassThroughHash<I> ph;
		I counter = 0;
		size_t optPrevention = 0;
		while (mb(title, optPrevention)) {
			optPrevention += ph(counter++);
		}

		boost::hash<I> bh;
		optPrevention = 0;
		counter = 0;
		msg = "boost::hash<" + titleStr + ">";
		title = msg.c_str();
		while (mb(title, optPrevention)) {
			optPrevention += bh(counter++);
		}
	}

	util::RobinHood::FastHash<I> fh;
	size_t optPrevention = 0;
	I counter = 0;
	std::string msg = "util::RobinHood::FastHash<" + titleStr + ">";
	const char* title = msg.c_str();

	while (mb(title, optPrevention)) {
		optPrevention += fh(counter++);
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_hashUint32) {
	benchHash<apr_uint32_t>(100 * 1000 * 1000, "apr_uint32_t");
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_hashUint64) {
	benchHash<apr_uint64_t>(100 * 1000 * 1000, "apr_uint64_t");
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_hashString) {
	const std::string v = "This is a test string for hashing";
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(20 * 1000 * 1000);

	if (mb.isAutoTimed()) {
		boost::hash<std::string> bh;
		size_t optPrevention = 0;
		while (mb("boost::hash<std::string>", optPrevention)) {
			optPrevention += bh(v);
		}
	}

	util::RobinHood::FastHash<std::string> fh;
	size_t optPrevention = 0;
	while (mb("util::RobinHood::FastHash<std::string>", optPrevention)) {
		optPrevention += fh(v);
	}
}

TEST(RobinHoodMapTest, testCalcMaxLoadFactor128) {
	using util::RobinHood::calcMaxLoadFactor128;

	for (float f = 0.0f; f < 2.0f; f += 0.01f) {
		size_t r = calcMaxLoadFactor128(f);
		REQUIRE(static_cast<size_t>(0) <= r);
		REQUIRE(static_cast<size_t>(128) >= r);
	}

	REQUIRE(calcMaxLoadFactor128(0.0f) == static_cast<size_t>(0));
	REQUIRE(calcMaxLoadFactor128(2.0f) == static_cast<size_t>(128));
	REQUIRE(calcMaxLoadFactor128(0.5f) == static_cast<size_t>(128 / 2));
	REQUIRE(calcMaxLoadFactor128(0.8f) == static_cast<size_t>(102));
}

TEST(RobinHoodMapTest, testCalcMaxNumElementsAllowed128) {
	using util::RobinHood::calcMaxNumElementsAllowed128;

	for (size_t maxElements = 1; maxElements > 0; maxElements *= 2) {
		for (unsigned char maxLoadFactor128 = 0; maxLoadFactor128 <= 128; ++maxLoadFactor128) {
			REQUIRE(maxLoadFactor128) == static_cast<size_t>(maxElements * (maxLoadFactor128 / 128.0)), calcMaxNumElementsAllowed128(maxElements);
		}
	}
}

TEST(RobinHoodMapTest, testCount) {
	util::RobinHood::Map<int, int> rh;
	boost::unordered_map<int, int> uo;
	REQUIRE(rh.count(123) == uo.count(123));
	REQUIRE(rh.count(0) == uo.count(0));
	rh[123];
	uo[123];
	REQUIRE(rh.count(123) == uo.count(123));
	REQUIRE(rh.count(0) == uo.count(0));
}

struct MyEquals : private boost::noncopyable {
	int& mState;

	MyEquals(int& state)
		: mState(state) {}

	MyEquals(const MyEquals& o)
		: mState(o.mState) {}

	bool operator()(const int& a, const int& b) const {
		std::cout << "equals with " << mState << std::endl;
		// modify state so we can test that this acutally works
		mState++;

		return a == b;
	}
};

TEST(RobinHoodMapTest, testCustomEquals) {
	int state = 0;
	MyEquals eq(state);
	typedef util::RobinHood::Map<int, int, util::RobinHood::FastHash<int>, MyEquals> MyMap;

	MyMap rh(0, util::RobinHood::FastHash<int>(), eq);

	// insert, no equals call
	rh[123] = 321;
	REQUIRE(state == 0);

	// lookup, should call equals once
	rh.find(123);
	REQUIRE(state == 1);

	// modify state ourselves
	state = 100;
	rh.find(123);
	REQUIRE(state == 101);

	// copy map, both should still use the same state
	MyMap rhCopy = rh;

	rhCopy.find(123);
	REQUIRE(state == 102);
	rh.find(123);
	REQUIRE(state == 103);

	// can't swap, deleted function.
}

template <class Map>
void testCollisions() {
	{
		Map m;
		for (int i = 0; i < 255; ++i) {
			m[i];
		}
		REQUIRE(m.size() == (size_t)255);
		REQUIRE_THROWS_AS(m[255], std::overflow_error);
		REQUIRE(m.size() == (size_t)255);
	}
	if (0 != CtorDtorVerifier::mapSize()) {
		CtorDtorVerifier::printMap();
	}
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);

	{
		Map m;
		for (int i = 0; i < 255; ++i) {
			REQUIRE(m.insert(typename Map::value_type(i, i)).second);
		}
		REQUIRE(m.size() == (size_t)255);
		REQUIRE_THROWS_AS(m.insert(typename Map::value_type(255, 255)), std::overflow_error);
		REQUIRE(m.size() == (size_t)255);
	}
	if (0 != CtorDtorVerifier::mapSize()) {
		CtorDtorVerifier::printMap();
	}
	REQUIRE(CtorDtorVerifier::mapSize() == (size_t)0);
}

TEST(RobinHoodMapTest, testCollisionSmall) {
	testCollisions<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, DummyHash<CtorDtorVerifier>, std::equal_to<CtorDtorVerifier>, true>>();
}
TEST(RobinHoodMapTest, testCollisionBig) {
	testCollisions<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, DummyHash<CtorDtorVerifier>, std::equal_to<CtorDtorVerifier>, false>>();
}

TEST(RobinHoodMapTest, testSmallTypes) {
	util::RobinHood::Map<uint8_t, uint8_t> map8;
	map8['x'] = 'a';
	REQUIRE(map8['x'] == 'a');
	REQUIRE(map8.size() == (size_t)1);

	util::RobinHood::Map<uint16_t, uint16_t> map16;
	map16[23] = 123;
	REQUIRE(map16[23] == (uint8_t)123);
	REQUIRE(map16.size() == (size_t)1);
}

template <class Map>
void eraseEven(Map& m) {
	typename Map::iterator it = m.begin();
	typename Map::iterator end = m.end();
	while (it != end) {
		// remove every second value (randomly)
		if (it->second % 2 == 0) {
			it = m.erase(it);
		} else {
			++it;
		}
	}
}

template <class A, class B>
void assertMapEqDirected(const A& a, const B& b) {
	REQUIRE(b.size() == a.size());
	for (typename A::const_iterator ia = a.begin(), aend = a.end(); ia != aend; ++ia) {
		typename B::const_iterator ib = b.find(ia->first);
		REQUIRE(ib != b.end());
		REQUIRE(ib->first == ia->first);
		REQUIRE(ib->second == ia->second);
	}
}

template <class A, class B>
void assertMapEq(const A& a, const B& b) {
	assertMapEqDirected(a, b);
	assertMapEqDirected(b, a);
}

TEST(RobinHoodMapTest, testErase) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	Map rh;
	boost::mt19937 gen;

	gen.seed(123);
	for (int i = 0; i < 10000; ++i) {
		rh[gen()] = gen();
	}
	std::cout << rh.size() << std::endl;
	eraseEven(rh);
	std::cout << rh.size() << std::endl;

	boost::unordered_map<uint32_t, uint32_t> uo;
	gen.seed(123);
	for (int i = 0; i < 10000; ++i) {
		uo[gen()] = gen();
	}
	std::cout << uo.size() << std::endl;
	eraseEven(uo);
	std::cout << uo.size() << std::endl;
	assertMapEq(rh, uo);
}

#ifdef BOOST_HAS_RVALUE_REFS
TEST(RobinHoodMapTest, testMoves) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	boost::scoped_ptr<Map> rh(new Map());
	(*rh)[123] = 321;

	Map rh2(std::move(*rh));
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);

	// destroy rh, everything should still be fine.
	rh.reset();
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);
}

TEST(RobinHoodMapTest, testMoveAssignment) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	boost::scoped_ptr<Map> rh(new Map());
	(*rh)[123] = 321;

	Map rh2;
	rh2[444] = 555;

	rh2 = std::move(*rh);
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);

	rh.reset();
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);
}

template <class Map>
void benchMoveMap(util::MicroBenchmark& mb, const char* title) {
	Map m1;
	Map m2;
	for (int i = 0; i < 10000; ++i) {
		m1[i];
	}

	while (mb(title)) {
		m2 = std::move(m1);
		m1 = std::move(m2);
	}

	REQUIRE(10000 == m1.size());
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_moveMap) {
	util::MicroBenchmark mb;
	mb.unitScaling("move", (size_t)2);

	mb.fixedIterationsPerMeasurement(20 * 1000 * 1000);
	if (mb.isAutoTimed()) {
		benchMoveMap<boost::unordered_map<int, BigObject>>(mb, "boost::unordered_map<int, BigObject>");
	}

	benchMoveMap<util::RobinHood::Map<int, BigObject>>(mb, "util::RobinHood::Map<int, BigObject>");
}
#endif

template <class M>
void assertEq(const M& a, const M& b) {
	REQUIRE(a == b);
	REQUIRE(b == a);
	REQUIRE(!(a != b));
	REQUIRE(!(b != a));
}

template <class M>
void assertNeq(const M& a, const M& b) {
	REQUIRE(a != b);
	REQUIRE(b != a);
	REQUIRE(!(a == b));
	REQUIRE(!(b == a));
}

TEST(RobinHoodMapTest, testCompare) {
	util::RobinHood::Map<std::string, int> a;
	util::RobinHood::Map<std::string, int> b;
	assertEq(a, b);

	a["123"] = 321;
	assertNeq(a, b);

	b["123"] = 321;
	assertEq(a, b);

	b["123"] = 320;
	assertNeq(a, b);

	b["123"] = 321;
	assertEq(a, b);

	b.clear();
	b["124"] = 321;
	assertNeq(a, b);

	a.clear();
	assertEq(a, util::RobinHood::Map<std::string, int>());

	a[""] = 0;
	assertNeq(a, b);
}

#include <boost/assign/list_of.hpp> // for 'map_list_of()'

TEST(RobinHoodMapTest, testAssignListOf) {
	util::RobinHood::Map<std::string, int> m = boost::assign::map_list_of("a", 1)("b", 2);
	REQUIRE(m.size() == (size_t)2);
}

TEST(RobinHoodMapTest, testFindOtherType) {
	util::RobinHood::Map<std::string, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> m;
	m["asdf"] = 123;

	REQUIRE(m.end() != m.find(boost::string_view("asdf"), util::RobinHood::is_transparent_tag()));
	REQUIRE(m.end() != m.find("asdf", util::RobinHood::is_transparent_tag()));
	REQUIRE(m.end() == m.find(boost::string_view("asdx"), util::RobinHood::is_transparent_tag()));
}

#include <util/typed/String.h>
#include <util/typed/StringView.h>

// util::typed::String<util::typed::CCSID_UTF8>
// util::typed::StringView<util::typed::CCSID_UTF8>
template <class M, class Str, class View>
void benchStringQuery(M& m, util::MicroBenchmark& mb, const char* title, util::RobinHood::is_transparent_tag) {
	Str str("This is a dummy test string that is also the query. It's a bit long.");
	for (size_t i = 0; i < 127; ++i) {
		++str[5];
		m[str] = 123;
	}

	View sv(str);
	size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		if (m.end() != m.find(sv, util::RobinHood::is_transparent_tag())) {
			++optPrevention;
		}
		++str[5];
	}
}

template <class M, class Str, class View>
void benchStringQuery(M& m, util::MicroBenchmark& mb, const char* title) {
	Str str("This is a dummy test string that is also the query. It's a bit long.");
	for (size_t i = 0; i < 127; ++i) {
		++str[5];
		m[str] = 123;
	}

	View sv(str);
	size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		if (m.end() != m.find(sv)) {
			++optPrevention;
		}
		++str[5];
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_testBoostStringView) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);
	{
		// does not compile:
		//	typedef boost::unordered_map<std::string, int> Map;
		//	typedef util::RobinHood::Map<std::string, int> Map;

		// but this does:
		typedef util::RobinHood::Map<std::string, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> Map;
		Map map;
		benchStringQuery<Map, std::string, boost::string_view>(map, mb, "boost::string_view fast", util::RobinHood::is_transparent_tag());
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_testPerformanceStringSearchIsTransparentTag) {
	typedef util::typed::String<util::typed::CCSID_UTF8> Str;
	typedef util::typed::StringView<util::typed::CCSID_UTF8> View;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);

	{
		typedef util::RobinHood::Map<Str, int> Map;
		Map m;
		benchStringQuery<Map, Str, View>(m, mb, "util::RobinHood::Map<Str, int>, int>", util::RobinHood::is_transparent_tag());
	}
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_testPerformanceStringSearch) {
	typedef util::typed::String<util::typed::CCSID_UTF8> Str;
	typedef util::typed::StringView<util::typed::CCSID_UTF8> View;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);

	{
		typedef util::RobinHood::Map<Str, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> Map;
		Map m;
		benchStringQuery<Map, Str, View>(m, mb, "util::RobinHood::Map<Str, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<> >",
										 util::RobinHood::is_transparent_tag());
	}
}

TEST(RobinHoodMapTest, testFindTyped) {
	// use FastHash<> and equal_to<> to enable the find() operation which does not copy.
	util::RobinHood::Map<util::typed::String<util::typed::CCSID_ASCII7>, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> m;

	util::typed::String<util::typed::CCSID_ASCII7> str("asdf");
	m[str] = 123;

	util::typed::StringView<util::typed::CCSID_ASCII7> sv(str);
	REQUIRE(m.end() != m.find(sv));
}

/*


#include "Windows.h"
#include <psapi.h>

#include "util/AprThread.h"

#include <boost/bind.hpp>

class PeriodicMemoryStats : private boost::noncopyable {
public:
	struct Stats {
		apr_time_t timestamp;
		size_t memPrivateUsage;
	};

	PeriodicMemoryStats(double intervalSeconds)
		: mIntervalMicros(static_cast<uint64_t>(intervalSeconds * 1000 * 1000))
		, mDoContinue(true)
		, mThread(util::thread::runAsync(boost::bind(&PeriodicMemoryStats::runner, this, _1), (void*)0)) {}

	~PeriodicMemoryStats() {
		stop();
	}

	void stop() {
		mDoContinue = false;
		if (mThread) {
			mThread->join();
		}
	}

	void event(const char* title) {
		Stats s;
		s.timestamp = apr_time_now();
		s.memPrivateUsage = getMem();
		mEvents.push_back(std::pair<Stats, std::string>(s, title));
	}

	friend std::ostream& operator<<(std::ostream& stream, const PeriodicMemoryStats& pms);

private:
	apr_status_t runner(void*) {
		apr_time_t nextStop = apr_time_now();

		Stats s;
		while (mDoContinue) {
			nextStop += mIntervalMicros;

			s.timestamp = apr_time_now();
			s.memPrivateUsage = getMem();
			mPeriodic.push_back(s);

			if (nextStop < s.timestamp) {
				// we can't keep up!
				nextStop = s.timestamp;
			}
			util::thread::sleep_for_usec(nextStop - s.timestamp);
		}

		// add one last measurement
		s.timestamp = apr_time_now();
		s.memPrivateUsage = getMem();
		mPeriodic.push_back(s);

		return APR_SUCCESS;
	}

	size_t getMem() {
		PROCESS_MEMORY_COUNTERS_EX info;
		info.cb = sizeof(info);
		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&info, info.cb)) {
			return info.PrivateUsage;
		}
		return 0;
	}

	std::vector<Stats> mPeriodic;
	std::vector<std::pair<Stats, std::string>> mEvents;

	uint64_t mIntervalMicros;
	bool mDoContinue;

	boost::scoped_ptr<util::AprThread<void*>> mThread;
};

inline std::ostream& operator<<(std::ostream& stream, const PeriodicMemoryStats& pms) {
	apr_time_t begin = pms.mPeriodic[0].timestamp;
	if (!pms.mEvents.empty()) {
		begin = (std::min)(begin, pms.mEvents[0].first.timestamp);
		for (size_t i = 0; i < pms.mEvents.size(); ++i) {
			const PeriodicMemoryStats::Stats& s = pms.mEvents[i].first;
			stream << (s.timestamp - begin) / 1000000.0 << "; " << s.memPrivateUsage << "; " << pms.mEvents[i].second << std::endl;
		}
		stream << std::endl;
	}

	for (size_t i = 0; i < pms.mPeriodic.size(); ++i) {
		const PeriodicMemoryStats::Stats& s = pms.mPeriodic[i];
		stream << (s.timestamp - begin) / 1000000.0 << "; " << s.memPrivateUsage << std::endl;
	}
	return stream;
}

#include "khash.h"

//#define my_size_t_hash(key) (khint32_t)(util::RobinHood::internal_hasher::fmix64(key))
// KHASH_INIT(map_size_t_size_t, khint64_t, size_t, 1, my_size_t_hash, kh_int64_hash_equal)
KHASH_MAP_INIT_INT64(map_size_t_size_t, size_t);
KHASH_MAP_INIT_INT64(map_size_t_BigObject, BigObject);

class KHashSizeT {
public:
	KHashSizeT()
		: mMap(kh_init(map_size_t_size_t)) {}

	~KHashSizeT() {
		kh_destroy(map_size_t_size_t, mMap);
	}

	size_t& operator[](const size_t& key) {
		int absent;
		auto k = kh_put(map_size_t_size_t, mMap, key, &absent);
		return kh_value(mMap, k);
	}

	size_t count(size_t s) const {
		if (kh_end(mMap) == kh_get(map_size_t_size_t, mMap, s)) {
			return 0;
		}
		return 1;
	}

	size_t size() const {
		return kh_size(mMap);
	}

	void clear() {
		kh_clear(map_size_t_size_t, mMap);
	}

private:
	khash_t(map_size_t_size_t) * mMap;
};

class KHashBigObject {
public:
	KHashBigObject()
		: mMap(kh_init(map_size_t_BigObject)) {}

	~KHashBigObject() {
		kh_destroy(map_size_t_BigObject, mMap);
	}

	BigObject& operator[](const size_t& key) {
		int absent;
		auto k = kh_put(map_size_t_BigObject, mMap, key, &absent);
		return kh_value(mMap, k);
	}

	size_t count(size_t s) const {
		if (kh_end(mMap) == kh_get(map_size_t_BigObject, mMap, s)) {
			return 0;
		}
		return 1;
	}

	size_t size() const {
		return kh_size(mMap);
	}

	void clear() {
		kh_clear(map_size_t_BigObject, mMap);
	}

private:
	khash_t(map_size_t_BigObject) * mMap;
};


template <class Map>
void benchRamUsage(const size_t numIters) {

	boost::mt19937 gen;
	gen.seed(123);

	size_t c = 0;
	PeriodicMemoryStats stats(0.02);
	{
		Map map;
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		stats.event("inserted");

		map.clear();
		stats.event("cleared");

		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		stats.event("inserted");
	}
	stats.event("destructed");
	stats.stop();

	std::cout << stats << std::endl;
	std::cout << c << std::endl;
}

template <class Map>
void benchCreateClearDestroy(size_t numIters) {
	boost::mt19937 gen;
	gen.seed(123);

	size_t c = 0;
	{
		Map map;
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		map.clear();
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
	}
	std::cout << c << std::endl;
}

TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearRobinHoodBig) {
	benchRamUsage<util::RobinHood::Map<size_t, BigObject>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdUnorderedMapBig) {
	benchRamUsage<std::unordered_map<size_t, BigObject>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearBoostUnorderedMapBig) {
	benchRamUsage<boost::unordered_map<size_t, BigObject>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdMapBig) {
	benchRamUsage<std::map<size_t, BigObject>>(10000000);
}

TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearRobinHoodSizeT) {
	benchRamUsage<util::RobinHood::Map<size_t, size_t>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdUnorderedMapSizeT) {
	benchRamUsage<std::unordered_map<size_t, size_t>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearBoostUnorderedMapSizeT) {
	benchRamUsage<boost::unordered_map<size_t, size_t>>(10000000);
}
TEST(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdMapSizeT) {
	benchRamUsage<std::map<size_t, size_t>>(10000000);
}

TEST(RobinHoodMapTest, DISABLED_testRamUsage) {
	// typedef util::RobinHood::Map<size_t, size_t> Map;
	// typedef util::RobinHood::Map<size_t, size_t, util::RobinHood::FastHash<size_t>, std::equal_to<size_t>, false> Map;
	// typedef util::RobinHood::Map<size_t, size_t, boost::hash<size_t>, std::equal_to<size_t>, false> Map;
	// typedef util::RobinHood::Map<size_t, size_t, boost::hash<size_t>, std::equal_to<size_t>, true> Map;
	// typedef std::map<size_t, size_t> Map;
	// typedef boost::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t> > Map;
	// typedef boost::unordered_map<size_t, size_t> Map;
	// typedef KHashBigObject Map;

	// typedef util::RobinHood::Map<size_t, BigObject> Map;
	// typedef util::RobinHood::IndirectMap<size_t, BigObject> Map;
	// typedef boost::unordered_map<size_t, BigObject> Map;
	// typedef std::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t> > Map;

	typedef util::RobinHood::IndirectMap<size_t, size_t> Map;
	benchRamUsage<Map>(20000000);
}
*/

TEST(RobinHoodMapTest, testEmplace) {
	util::RobinHood::Map<std::string, std::string> map;
	auto itAndInserted = map.emplace("key", "val");
	REQUIRE(itAndInserted.second);
	REQUIRE(itAndInserted.first != map.end());
	REQUIRE(itAndInserted.first->first == "key");
	REQUIRE(itAndInserted.first->second == "val");

	// insert fails, returns iterator to previously inserted element
	itAndInserted = map.emplace("key", "val2");
	REQUIRE(!itAndInserted.second);
	REQUIRE(itAndInserted.first != map.end());
	REQUIRE(itAndInserted.first->first == "key");
	REQUIRE(itAndInserted.first->second == "val");
}

TEST(RobinHoodMapTest, testStringMoved) {
	util::RobinHood::Map<std::string, std::string> map;
	map["foo"] = "bar";
}

TEST(RobinHoodMapTest, testAssignIterat) {
	const std::string key = "key";
	util::RobinHood::Map<std::string, std::string> map;
	map[key] = "sadf";
	auto it = map.end();
	it = map.end();

	auto cit = map.cend();
	cit = map.cend();

	cit = it;
}
/*
template <class M>
void testInitializerList() {
	M m = {{123, "Hello"}, {321, "Hugo"}, {123, "Goodbye"}, {}};
	REQUIRE(m.size() == (size_t)3);
	REQUIRE(m[123] == "Hello");
	REQUIRE(m[321] == "Hugo");
	REQUIRE(m[0] == "");
	REQUIRE(m.size() == (size_t)3);
}

TEST(RobinHoodMapTest, testInitializerList) {
	testInitializerList<std::map<int, std::string>>();
	testInitializerList<util::RobinHood::Map<int, std::string>>();
}
*/
template <class Map>
void testBrackets() {
	resetStaticCounts();
	printStaticHeader();
	{
		Map map;
		printStaticCounts("empty");
		resetStaticCounts();
		for (size_t i = 0; i < 5; ++i) {
			map[321] = 123;
			printStaticCounts("map[321] = 123");
			resetStaticCounts();
		}
	}
	printStaticCounts("dtor");
	resetStaticCounts();
}

TEST(RobinHoodMapTest, testBracketsRobinHoodDirect) {
	testBrackets<util::RobinHood::DirectMap<Counter<int>, int>>();
}
TEST(RobinHoodMapTest, testBracketsRobinHoodIndirect) {
	testBrackets<util::RobinHood::IndirectMap<Counter<int>, int>>();
}
TEST(RobinHoodMapTest, testBracketsUnorderedMap) {
	testBrackets<std::unordered_map<Counter<int>, int>>();
}
TEST(RobinHoodMapTest, testBracketsMap) {
	testBrackets<std::map<Counter<int>, int>>();
}

template <class Map>
void benchStringQuery(util::MicroBenchmark& mb, const char* title) {
	Map m;
	std::string const str = "this is my query. It is a very nice query, but unfortunately quite long, so when it is copied this will be slow.";
	std::vector<std::string> strings;
	for (size_t i = 0; i < 100; ++i) {
		strings.push_back(str + std::to_string(i));
	}

	while (mb(title)) {
		for (size_t i = 0; i < strings.size(); ++i) {
			++m[strings[i]];
		}
	}
	mb.noOpt(m.size());
}

TEST(RobinHoodMapTest, PERFORMANCE_SLOW_accessExistingWithConstRef) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(20'000);
	mb.unitScaling("operator[]", 100ull);
	if (mb.isAutoTimed()) {
		benchStringQuery<std::unordered_map<std::string, size_t>>(mb, "std::unordered_map");
		benchStringQuery<boost::unordered_map<std::string, size_t>>(mb, "boost::unordered_map");
	}
	benchStringQuery<util::RobinHood::DirectMap<std::string, size_t>>(mb, "util::RobinHood::DirectMap");
	benchStringQuery<util::RobinHood::IndirectMap<std::string, size_t>>(mb, "util::RobinHood::IndirectMap");

	mb.plot();
}

TEST(RobinHoodMapTest, testCopyable) {
	static_assert(std::is_trivially_copyable<util::RobinHood::Pair<int, int>>::value, "NOT is_trivially_copyable");
	static_assert(std::is_trivially_destructible<util::RobinHood::Pair<int, int>>::value, "NOT is_trivially_destructible");
}

TEST(RobinHoodMapTest, testInsertList) {
	std::vector<util::RobinHood::Pair<int, int>> v;
	v.emplace_back(1, 2);
	v.emplace_back(3, 4);

	util::RobinHood::Map<int, int> map(v.begin(), v.end());
}

struct Hash {
	size_t operator()(uint64_t x) {
		return (size_t)x;
	}
};

TEST(RobinHoodMapTest, testCustomHash) {
	util::RobinHood::Map<uint64_t, int16_t, Hash> map;
}
