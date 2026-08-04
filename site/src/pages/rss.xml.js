import { getCollection } from 'astro:content';
import rss from '@astrojs/rss';
import { SITE_DESCRIPTION, SITE_TITLE } from '../consts';

export async function GET(context) {
	const posts = await getCollection('blog');
	// `base` must be folded in explicitly: @astrojs/rss resolves item links against
	// the origin, not against the site URL's path.
	const base = import.meta.env.BASE_URL.replace(/\/?$/, '/');
	const site = new URL(base, context.site);
	return rss({
		title: SITE_TITLE,
		description: SITE_DESCRIPTION,
		site,
		items: posts.map((post) => ({
			...post.data,
			link: new URL(`blog/${post.id}/`, site).href,
		})),
	});
}
