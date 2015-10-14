// different snipets of code for Codingame


// auto usage
for (auto iNd : aDoubleLinkNodes) {
}

for (auto &iNd : aDoubleLinkNodes) {
}

// lambda sort
s.sort([](const pair<string,int>& a, const pair<string,int>& b)
			{
			 return (a.second) > (b.second);
			});

for_each( v.begin(), v.end(), [] (int val)
{
    cout << val;
} );


