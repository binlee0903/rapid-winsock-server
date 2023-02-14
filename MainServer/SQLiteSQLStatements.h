#pragma once

namespace SQLiteSQLStatements
{
	constexpr char GetArticle[] = "SELECT * FROM ARTICLES WHERE \"INDEX\" = ?/1 ; ";
	constexpr char GetArticles[] = "SELECT \"INDEX\", \"TITLE\", \"TITLE_IMAGE_PATH\", \"DATE\" FROM ARTICLES LIMIT ?1, 6;";
	constexpr char Insert[] = "INSERT INTO ARTICLES (\"TITLE\", \"TITLE_IMAGE_PATH\", \"DATE\", \"ARTICLE\") VALUES (?, ?, ?, ?);";
	//constexpr char Update[] = "UPDATE ARTICLES SET CONTENT = ? WHERE \"INDEX\" = ?;";
	//constexpr char Delete[] = "DELETE FROM ARTICLES WHERE \"INDEX\" = ?;";
};