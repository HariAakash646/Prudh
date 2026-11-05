#include "cli.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>

namespace
{
constexpr std::array<const char *, 8> direction_names = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

std::string to_upper(std::string s)
{
    for (char &c : s)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool owned_by_player1(int row, int col) { return (row + col) % 2 == 0; }
} // namespace

CLI::CLI(std::istream &in, std::ostream &out) : in(in), out(out) {}

void CLI::run()
{
    out << "Prudh - the Gentlemen's Game\n";
    print_help();

    while (game.are_moves_left())
    {
        print_board();
        print_scores();
        out << player_name(game.get_turn()) << " to move > " << std::flush;

        std::string line;
        if (!std::getline(in, line))
            return; // EOF

        std::string cmd = to_upper(line);
        if (cmd == "QUIT" || cmd == "Q")
            return;
        if (cmd == "HELP" || cmd == "H")
        {
            print_help();
            continue;
        }
        if (cmd == "MOVES" || cmd == "M")
        {
            print_valid_moves();
            continue;
        }

        auto move = parse_move(line);
        if (!move)
            continue;
        if (!game.make_move(*move))
            out << "Illegal move. Type 'moves' to see what you can play.\n";
    }

    game.complete_game();
    print_board();
    print_result();
}

void CLI::print_board()
{
    auto grid = game.get_board();
    out << "\n     ";
    for (int c = 0; c < board_size; ++c)
        out << "  " << static_cast<char>('a' + c) << " ";
    out << '\n';

    for (int r = 0; r < board_size; ++r)
    {
        out << "  " << r + 1 << "  ";
        for (int c = 0; c < board_size; ++c)
        {
            // X = Player1's square, O = Player2's square, followed by stack height.
            char owner = owned_by_player1(r, c) ? 'X' : 'O';
            if (grid[r][c] == 0)
                out << "  . ";
            else
                out << ' ' << owner << grid[r][c] << (grid[r][c] < 10 ? " " : "");
        }
        out << '\n';
    }
    out << '\n';
}

void CLI::print_scores()
{
    out << "Score  " << player_name(Player1) << ": " << game.get_player_score(0)
        << "   " << player_name(Player2) << ": " << game.get_player_score(1) << '\n';
}

void CLI::print_help()
{
    out << "\nPlayer 1 owns the X squares, Player 2 owns the O squares.\n"
           "Enter a move as <cell> <direction>, e.g. 'b3 SE'.\n"
           "  cell: column a-f, row 1-6 (row 1 is the top)\n"
           "  slide: NE SE SW NW   run: N E S W\n"
           "Commands: moves (m), help (h), quit (q)\n";
}

void CLI::print_valid_moves()
{
    auto moves = game.get_valid_moves();
    out << moves.size() << " legal move(s):\n";
    int i = 0;
    for (const auto &m : moves)
    {
        out << "  " << cell_name(m.cell) << ' ' << direction_name(m.dir);
        if (++i % 6 == 0)
            out << '\n';
    }
    out << '\n';
}

void CLI::print_result()
{
    int p1 = game.get_player_score(0);
    int p2 = game.get_player_score(1);
    out << "Game over!\n";
    print_scores();

    // Ties go to the player who moved second (Player 2, since Player 1 always starts).
    out << (p1 > p2 ? player_name(Player1) : player_name(Player2)) << " wins";
    if (p1 == p2)
        out << " (tie goes to the player who went second)";
    out << "!\n";
}

std::optional<Move> CLI::parse_move(const std::string &line)
{
    std::istringstream ss(line);
    std::string cell_text, dir_text, extra;
    if (!(ss >> cell_text >> dir_text) || (ss >> extra))
    {
        out << "Expected '<cell> <direction>', e.g. 'b3 SE'. Type 'help' for more.\n";
        return std::nullopt;
    }

    char col_ch = static_cast<char>(std::tolower(static_cast<unsigned char>(cell_text[0])));
    int col = col_ch - 'a';
    int row = -1;
    if (cell_text.size() == 2 && std::isdigit(static_cast<unsigned char>(cell_text[1])))
        row = cell_text[1] - '1';
    if (col < 0 || col >= board_size || row < 0 || row >= board_size)
    {
        out << "Bad cell '" << cell_text << "'. Use a-f followed by 1-6, e.g. 'b3'.\n";
        return std::nullopt;
    }

    std::string dir_upper = to_upper(dir_text);
    auto it = std::ranges::find(direction_names, dir_upper);
    if (it == direction_names.end())
    {
        out << "Bad direction '" << dir_text << "'. Use N NE E SE S SW W NW.\n";
        return std::nullopt;
    }

    return Move{{row, col}, static_cast<Direction>(it - direction_names.begin())};
}

std::string CLI::cell_name(const Cell &cell)
{
    return {static_cast<char>('a' + cell.col), static_cast<char>('1' + cell.row)};
}

std::string CLI::direction_name(Direction dir)
{
    return direction_names[dir];
}

const char *CLI::player_name(Turn turn)
{
    return turn == Player1 ? "Player 1 (X)" : "Player 2 (O)";
}
