#pragma once

namespace SQLiteSQLStatements
{
	constexpr char Select[] = "SELECT * FROM ? WHERE INDEX = ?;";
	constexpr char Insert[] = "INSERT INTO ? VALUES (?);";
	constexpr char Update[] = "UPDATE ? SET CONTENT = ? WHERE INDEX = ?;";
	constexpr char Delete[] = "DELETE FROM ? WHERE INDEX = ?;";
};