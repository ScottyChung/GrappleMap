#include "util.hpp"
#include "graph.hpp"

Graph::Graph(std::vector<Sequence> const & sequences)
{
	foreach (s : sequences) insert(s);
	std::cerr << "Loaded " << nodes.size() << " nodes and " << edges.size() << " edges." << std::endl;
}

void Graph::changed(PositionInSequence const pis)
{
	Edge & edge = edges.at(pis.sequence);

	if (pis.position == 0)
	{
		auto const new_from = find_or_add(edge.sequence.positions.front());

		if (new_from.node != edge.from.node)
			std::cerr << "Front of sequence is now a different node." << std::endl;

		edge.from = new_from;
	}
	else if (!next(*this, pis))
	{
		auto const new_to = find_or_add(edge.sequence.positions.back());

		if (new_to.node != edge.to.node)
			std::cerr << "Back of sequence is now a different node." << std::endl;

		edge.to = new_to;
	}
}

void Graph::replace(PositionInSequence const pis, Position const & p)
{
	edges.at(pis.sequence).sequence.positions.at(pis.position) = p;
	changed(pis);

	std::cerr << "Replaced position " << pis << std::endl;
}

void Graph::clone(PositionInSequence const pis)
{
	auto & positions =  edges.at(pis.sequence).sequence.positions;
	Position const p = positions.at(pis.position);
	positions.insert(positions.begin() + pis.position, p);
}

SeqNum Graph::insert(Sequence const & sequence)
{
	auto const num = edges.size();

	std::cerr <<
		"Inserted sequence " << num <<
		" (\"" << sequence.description << "\")"
		" of size " << sequence.positions.size() << std::endl;

	edges.push_back({
		find_or_add(sequence.positions.front()),
		find_or_add(sequence.positions.back()),
		sequence});

	return num;
}

boost::optional<SeqNum> Graph::erase_sequence(SeqNum const sn)
{
	if (edges.size() == 1)
	{
		std::cerr << "Cannot erase last sequence." << std::endl;
		return boost::none;
	}

	auto const & seq = sequence(sn);

	std::cerr <<
		"Erasing sequence " << sn <<
		" (\"" << seq.description << "\")"
		" and the " << seq.positions.size() << " positions in it." << std::endl;

	edges.erase(edges.begin() + sn);

	return sn == 0 ? 0 : sn - 1;
}

boost::optional<PosNum> Graph::erase(PositionInSequence const pis)
{
	auto & edge = edges.at(pis.sequence);
	auto & positions = edge.sequence.positions;

	if (positions.size() == 2)
	{
		std::cerr << "Cannot erase either of last two elements in sequence." << std::endl;
		return boost::none;
	}

	positions.erase(positions.begin() + pis.position);

	auto const pos = std::min(pis.position, last_pos(*this, pis.sequence));

	changed({pis.sequence, pos});

	return pos;
}

boost::optional<SeqNum> seq_by_desc(Graph const & graph, std::string const & desc)
{
	for (SeqNum seqNum = 0; seqNum != graph.num_sequences(); ++seqNum)
		if (graph.sequence(seqNum).description == desc)
			return seqNum;
	
	return boost::none;
}
