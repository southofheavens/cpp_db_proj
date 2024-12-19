--
-- PostgreSQL database dump
--

-- Dumped from database version 17.1
-- Dumped by pg_dump version 17.2 (Homebrew)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: delete_guitar_attributes_func(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.delete_guitar_attributes_func() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
  DELETE FROM guitar_attributes WHERE guitar_attr_id = OLD.guitar_attr_id;
  RETURN NULL;
EXCEPTION
  WHEN OTHERS THEN
    RAISE NOTICE 'Ошибка при удалении из guitar_attributes: %', SQLERRM;
    RETURN NULL; -- Или можно вернуть OLD, чтобы отменить удаление гитары
END;
$$;


ALTER FUNCTION public.delete_guitar_attributes_func() OWNER TO postgres;

--
-- Name: get_guitars_by_category(integer); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.get_guitars_by_category(p_category_id integer) RETURNS TABLE(path_to_preview text, guitar_attr_id bigint, name character varying, price numeric, path_to_photo text)
    LANGUAGE plpgsql
    AS $$
BEGIN
    RETURN QUERY
    SELECT 
        g.path_to_preview,
        g.guitar_attr_id,
        g.name,
        g.price,
        gc.path_to_photo
    FROM 
        guitars AS g
    JOIN 
        guitar_categories AS gc ON g.category_id = gc.category_id
    WHERE 
        g.category_id = p_category_id;
END;
$$;


ALTER FUNCTION public.get_guitars_by_category(p_category_id integer) OWNER TO postgres;

--
-- Name: process_order_items(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.process_order_items() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Обновление количества гитар в БД
    UPDATE guitar_attributes AS ga
    SET in_stock = in_stock - NEW.quantity
    FROM guitars AS g
    WHERE g.guitar_attr_id = ga.guitar_attr_id 
      AND g.guitar_id = NEW.guitar_id;

    -- Удаление товара из корзины
    DELETE FROM cart_items 
    WHERE guitar_id = NEW.guitar_id 
      AND user_id = NEW.user_id;

    RETURN NEW;
END;
$$;


ALTER FUNCTION public.process_order_items() OWNER TO postgres;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: cart_items; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.cart_items (
    cart_item_id bigint NOT NULL,
    guitar_id bigint NOT NULL,
    quantity bigint NOT NULL,
    user_id bigint NOT NULL
);


ALTER TABLE public.cart_items OWNER TO postgres;

--
-- Name: cart_items_cart_item_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.cart_items ALTER COLUMN cart_item_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.cart_items_cart_item_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: guitar_attributes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.guitar_attributes (
    guitar_attr_id bigint NOT NULL,
    description text NOT NULL,
    num_of_strings integer NOT NULL,
    design character varying(255) NOT NULL,
    scale character varying(255) NOT NULL,
    frame character varying(255) NOT NULL,
    neck character varying(255) NOT NULL,
    fingerboard character varying(255) NOT NULL,
    fingerboard_radius integer NOT NULL,
    color character varying(255) NOT NULL,
    nut_width integer NOT NULL,
    pegs character varying(255) NOT NULL,
    fingerboard_sensor character varying(255),
    bridge_sensor character varying(255) NOT NULL,
    electronics character varying(255) NOT NULL,
    strings character varying(255),
    case_included boolean NOT NULL,
    country_of_origin character varying(255),
    in_stock integer NOT NULL,
    path_to_photo text NOT NULL
);


ALTER TABLE public.guitar_attributes OWNER TO postgres;

--
-- Name: guitar_attributes_guitar_attr_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.guitar_attributes ALTER COLUMN guitar_attr_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.guitar_attributes_guitar_attr_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: guitar_categories; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.guitar_categories (
    category_id bigint NOT NULL,
    category_name character varying(255) NOT NULL,
    path_to_preview text NOT NULL,
    path_to_photo text NOT NULL,
    CONSTRAINT guitar_categories_category_name_check CHECK (((category_name)::text = ANY ((ARRAY['acoustic'::character varying, 'bass'::character varying, 'electro'::character varying])::text[])))
);


ALTER TABLE public.guitar_categories OWNER TO postgres;

--
-- Name: guitar_categories_category_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.guitar_categories ALTER COLUMN category_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.guitar_categories_category_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: guitars; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.guitars (
    guitar_id bigint NOT NULL,
    category_id bigint NOT NULL,
    article character varying(255) NOT NULL,
    name character varying(255) NOT NULL,
    price numeric(10,2) NOT NULL,
    path_to_preview text NOT NULL,
    guitar_attr_id bigint
);


ALTER TABLE public.guitars OWNER TO postgres;

--
-- Name: guitar_details_view; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.guitar_details_view AS
 SELECT ga.description,
    ga.num_of_strings,
    ga.design,
    ga.scale,
    ga.frame,
    ga.neck,
    ga.fingerboard,
    ga.fingerboard_radius,
    ga.color,
    ga.nut_width,
    ga.pegs,
    ga.fingerboard_sensor,
    ga.bridge_sensor,
    ga.electronics,
    ga.strings,
    ga.case_included,
    ga.country_of_origin,
    ga.in_stock,
    ga.path_to_photo,
    g.guitar_id,
    ga.guitar_attr_id
   FROM (public.guitars g
     JOIN public.guitar_attributes ga ON ((g.guitar_attr_id = ga.guitar_attr_id)));


ALTER VIEW public.guitar_details_view OWNER TO postgres;

--
-- Name: guitars_guitar_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.guitars ALTER COLUMN guitar_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.guitars_guitar_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: order_items; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.order_items (
    order_item_id bigint NOT NULL,
    guitar_id bigint NOT NULL,
    order_id bigint NOT NULL,
    quantity integer NOT NULL,
    user_id bigint NOT NULL
);


ALTER TABLE public.order_items OWNER TO postgres;

--
-- Name: order_items_order_item_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.order_items ALTER COLUMN order_item_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.order_items_order_item_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: orders; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.orders (
    order_id bigint NOT NULL,
    user_id bigint NOT NULL,
    order_date timestamp without time zone NOT NULL,
    shipping_address character(255) NOT NULL,
    status character varying(255) NOT NULL,
    CONSTRAINT check_status CHECK (((status)::text = ANY ((ARRAY['created'::character varying, 'in process'::character varying, 'delivered'::character varying])::text[])))
);


ALTER TABLE public.orders OWNER TO postgres;

--
-- Name: orders_order_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.orders ALTER COLUMN order_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.orders_order_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: users; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.users (
    user_id integer NOT NULL,
    type character varying(5) NOT NULL,
    login character varying(255) NOT NULL,
    password text NOT NULL,
    email character varying(255) NOT NULL,
    phone_number character varying(20) NOT NULL,
    name character varying(255) NOT NULL,
    CONSTRAINT users_type_check CHECK (((type)::text = ANY ((ARRAY['user'::character varying, 'admin'::character varying])::text[])))
);


ALTER TABLE public.users OWNER TO postgres;

--
-- Name: users_user_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.users_user_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.users_user_id_seq OWNER TO postgres;

--
-- Name: users_user_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.users_user_id_seq OWNED BY public.users.user_id;


--
-- Name: users user_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users ALTER COLUMN user_id SET DEFAULT nextval('public.users_user_id_seq'::regclass);


--
-- Data for Name: cart_items; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.cart_items (cart_item_id, guitar_id, quantity, user_id) FROM stdin;
\.


--
-- Data for Name: guitar_attributes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.guitar_attributes (guitar_attr_id, description, num_of_strings, design, scale, frame, neck, fingerboard, fingerboard_radius, color, nut_width, pegs, fingerboard_sensor, bridge_sensor, electronics, strings, case_included, country_of_origin, in_stock, path_to_photo) FROM stdin;
6	LTD EC-1000FR BLKS (BLACK SATIN) электрогитара 6-струнная с флойдом.\n\nEC-1000FR BLKS, Электрогитара, корпус махогани/клен, вклеенный гриф махогани/палисандр 24.75", EMG-81/60, Floyd Rose, 24 лада, цвет Black Satin, LTD.\n\nГитары серии LTD Deluxe EC-1000 созданы для того, чтобы предложить звук, ощущение, внешний вид и качество, которые требуются от инструмента профессиональным музыкантам, оставаясь при этом достаточно доступными для серьезных музыкантов. Эта модель EC-1000FR оснащена бриджем Floyd Rose 1000SE с винтами из нержавеющей стали. Его набор активных звукоснимателей EMG 60 (нэк) и EMG 81 (бридж) обеспечивает отличное звучание. Он также включает в себя 24 лада из нержавеющей стали на накладке из черного дерева Macassar.	6	Глубоко вклеенный гриф	629 мм/24.75"	Красное дерево	Красное дерево, 3 куска\n	Macassar Ebony	350	Black Satin\n	42	Grover	EMG 60\n	EMG 81\n	Активная	D'Addario XL120 (.009 - .042)\n	f		1	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC-1000FR_BLKS(L).jpg
8	LTD EC-1000PIEZO QM STB 6-cтрунная электрогитара с пьезозвукоснимателем.\n\nСамая инновационная и универсальная электрогитара в серии LTD Deluxe, EC-1000 PIEZO — это первая серийная гитара LTD, оборудованная как стандартными магнитным звукоснимателями Seymour Duncan, так и совершенно отдельной системой с пьезо-датчиком. Пьезозвукосниматель создаёт совсем иное звучание — с отличной артикуляцией, больше похожее на звук акустической гитары. С помощью мини-переключателя вы можете переключаться между непревзойдёнными хамбакерами Seymour Duncan ('59 и JB) и датчиком Fishman Powerbridge, или даже комбинировать их! Многообразие вариантов звучания увеличивается ещё больше благодаря двум выходным гнездам Jack 6.3", которые позволяют выводить сигнал от разных датчиков в разные усилители, или интерфейсы, как на сцене, так и в студии. Это всё дополняется красивым топом из стёганного (Quilted) клёна в цвете See Thru Blue.	6	Глубоко вклеенный гриф\n	629 мм/24.75"\n	Красное дерево\n	Красное дерево, 3 куска\n	Пау ферро\n	350	See Thru Blue\n	42	Локовые LTD\n	Seymour Duncan '59\n	Seymour Duncan JB\n	Активная	D'Addario XL110 (.010 - .046)\n	f		1	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC-1000PIEZO_QM_STB(L).jpg
9	LTD HORIZON CTM '87 BLACK\nLTD HORIZON CUSTOM '87 BLK электрогитара 6 стр, корпус ольха, гриф Клён/Макассар, 24 л, 25,5", Floyd Rose 1000, Seymour Duncan Custom/Hotrail, цвет Black.\n\nESP впервые появилась в 1975 году как небольшая ремонтная мастерская в Токио, именно в середине 80-х наши гитары и бас-гитары привлекли внимание музыкантов по всему миру. Они расширили популярную серию LTD ’87, включив в нее некоторые из самых желанных гитар того времени. Horizon Custom ’87 с корпусом Archtop от ESP Horizon предлагает его именно таким, каким вы его нашли бы в конце 80-х и начале 90-х годов, минимально украшенный и созданный для того, чтобы удобно играть. Эта гитара имеет конструкцию «сквозной гриф» с мензурой 25,5 дюймов для интенсивного сустейна, сочетая корпус из ольхи и сверхтонкий трехкомпонентный кленовый гриф. Horizon Custom ’87 имеет накладку грифа из черного дерева макассар с 24 ладами extra-jumbo и без инкрустаций. В его состав входят: Floyd Rose 1000, Seymour Duncan TB-5 Custom Trembucker в бридже и Seymour Duncan Hot Rails в позиции грифа. 	6	Сквозной гриф\n	25,5 дюйма\n	Ольха	Клен	Черное дерево\n	350	Черный	42	С блокировкой\n	Seymour Duncan сингл\n	Хамбакер Seymour Duncan\n	Пассивная	D'Addario XL110 (.010 - .046)\n	t		3	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_HORIZON_CUSTOM_`87_BLK(L).jpg
11	LTD M-7HT BARITONE BLACK METAL BLKS (BLACK SATIN) электогитара баритон 7-струнная.\n\nСерия LTD Black Metal — это гитары, сравнимые по качеству сборки с нашими инструментами LTD Deluxe «1000 Series», но с темным и угрожающим дизайном: полностью черная отделка, компоненты и оборудование, один звукосниматель премиум-класса, накладки грифа из черного дерева без инкрустаций и светящихся в темноте боковых маркеров, и даже логотип LTD из черного металла на головке грифа. M-7HT Baritone Black Metal — это чудовищная 7-струнная гитара с мензурой 27 дюймов, которая предлагает сквозную конструкцию с корпусом из ольхи и сверхтонким U-образным кленовым грифом, состоящим из трех частей. Он оснащен бриджем Hipshot со струнами сквозь корпус для приятного сустейна и стабильности. Для брутального звучания и отличной динамики вы получаете одиночный пассивный звукосниматель Seymour Duncan Blackened Black Winter, с двухтактным управлением для разделения катушек.	7	Глубоко вклеенный гриф\n	686 мм (27')\n	Ольха	Клён (3 части)\n	Черное дерево\n	400	Black Satin\n	48	LTD локовые\n		Hipshot w/ String Thru\n	Пассивная	D'Addario XL110-7 (.010 - .059)\n	f		3	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-7HT_BARITONE_BLACK_METAL_BLKS(L).jpg
12	LTD M-1000 PURPLE NATURAL BURST электрогитара 6-струнная.\n\nЭлектрогитары ESP серии M за всю историю своего существования успели стать лицом компании. LTD M-1000 продолжает эту традицию, делая этот легендарный дизайн доступнее. Она предлагает серьёзным музыкантам конструкцию с глубоко вклеенным грифом профиля Extra Thin U из клёна с супер-отзывчивой накладкой из эбена с 24 ладами. Не менее отзывчивый корпус из ольхи дополняется двумя активными EMG 57TW/66TW с пуш-пулл переключателями на ручках громкости и тона для отсечки катушек каждого датчика, результатом чего является агрессивное и мощное звучание. Также на гитаре установлена тремоло-система Floyd Rose 1000SE. Гитара поставляется в красивом цвете Purple Natural Burst, который отлично сочетается с топом из капа тополя.	6	Глубоко вклеенный гриф\n	648 мм (25.5")\n	Ольха, топ - кап тополя\n	Клен, 3 части\n	Эбони	350	Purple Natural Burst\n	43	Grover	EMG 66TW\n	EMG 57TW\n	Активная	D'Addario XL120 (.009 - .042)\n	f		0	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-1000_PURPLE_NATURAL_BURST(L).jpg
15	M-200 FM See Thru Red.\n\nM-200 FM See Thru Red электрогитара 6 стр, корпус махогани, топ огненный клён, гриф на болтах, клён/ятоба, 24 л, 25.5", LTD by Floyd Rose, ESP Designed LH-150N/B.\n\nС гитарами M-200 гитаристы имеют действительно доступную и высококачественную гитару. Корпус классической плоской формы со скосом на этих гитарах выполнен из красного дерева, с грифом из клёна и накладкой из запечёной ятобы с 24 ладами extra-jumbo. Эти "скоростная" модель на болтах с тонким и плоским кленовым грифом, бриджем LTD by Floyd Rose и мощными датчиками ESP Designed.	6	На болтах\n	648 мм (25.5")\n	Красное дерево\n	Клён	Печёная ятоба\n	350	See Thru Red\n	43	LTD	ESP Designed LH-150N\n	ESP Designed LH-150B\n	Пассивная	D'Addario XL120 (.009/.011/.016/.024/.032/.042)\n	f		3	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/M-200_FM_See_Thru_Red(L).jpg
18	LTD D-320E NATURAL SATIN электроакустическая гитара 6-струнная.\n\nПри своей доступной цене, электроакустические гитары LTD обладают многими характеристиками более дорогих представителей: великолепный стильный внешний вид с подчеркнутой текстурой древесины и многослойной окантовкой, фирменная эргономика инструментов ESP и достойное акустическое звучание.	6	Вклеенный гриф\n	25,5"/648 мм\n	Овангкол/Массив ситхинской ели\n	Красное дерево\n	Печённая ятоба\n	350	Natural Satin\n	43	LTD	Fishman Sonicore\n	Fishman Sonicore\n	Fishman Presys I preamp w/ Onboard Tuner\n		f		0	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/acoustic/LTD_D-320E_NATURAL_SATIN(L).jpg
17	LTD B-55 FM STRSB бас-гитара 5-струнная.\n\nИнструменты линейки ESP LTD B Series представлены большим разнообразием конфигураций и оснащений бас-гитар: "сквозной" или привинченный гриф, 4, 5 или даже 6 струн, активная или пассивная электроника. В этой универсальной серии любой бас-гитарист найдет подходящий для себя инструмент, в том числе и по стоимости: от бюджетных моделей для начинающих, и до профессиональных инструментов. Разумеется, каджый инструмент серии LTD B выполнен на достойном уровне качества.	5	Привинченный гриф\n	864 мм\n	Липа	Клен	Палисандр	400	See Thru Red Sunburst\n	43	ESP LTD\n	ESP LDP\n	ESP LDJ\n	Активная	ESP BS-30HB\n	f		0	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/bass/LTD_B-55_FM_STRSB(L).jpg
13	LTD TE-1000 EVERTUNE CHARCOAL METALLIC SATIN электрогитара 6-струнная.\n\nГитары LTD Deluxe созданы для того, чтобы предложить звук, ощущения, внешний вид и качество, которые требуются от инструмента профессиональным музыкантам, оставаясь при этом достаточно доступными по цене. TE-1000 EverTune включает в себя мощный и инновационный бридж постоянного натяжения EverTune, гарантирующий, что почти в любых условиях ваша гитара будет оставаться в идеальной настройке и иметь невероятно точную интонацию в каждой точке накладки грифа. Эта гитара имеет сквозную конструкцию с мензурой 25,5 дюймов, корпус из красного дерева с кленовой накладкой и кленовый гриф из трех частей с накладкой из черного дерева Macassar и 24 ладами из нержавеющей стали экстра-джамбо. TE-1000 EverTune Charcoal Metallic Satin готов к серьезному року с активными звукоснимателями EMG 60TW-R (с push-pull) и EMG 81 (бридж).	6	Вклеенный гриф\n	648 мм (25.5")\n	Чёрное дерево с вставками клёна\n	Клен, 3 части\n	Чёрное дерево\n	350	Charcoal Metallic Satin\n	42	LTD locking\n	EMG 60TW-R Brushed Black Chrome *(Pull Tone for Split)\n	EMG 81 Brushed Black Chrome\n	Активная	D'Addario XL120 (.009 - .042)\n	f		2	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_TE-1000_EVERTUNE_CHARCOAL_METALLIC_SATIN(L).jpg
14	LTD TE-1000 EVERTUNE KOA NATURAL GLOSS электрогитара 6-струнная.\n\nГитары LTD Deluxe созданы для того, чтобы предложить звук, ощущения, внешний вид и качество, которые требуются от инструмента профессиональным музыкантам, оставаясь при этом достаточно доступными по цене. TE-1000 EverTune Koa выглядит фантастически благодаря своему корпусу из натурального дерева, покрытому гавайским КОА в отделке Natural Gloss. Бридж постоянного натяжения EverTune гарантирует, что почти в любых условиях ваша гитара будет оставаться в идеальном строе и иметь невероятно точную интонацию в каждой точке накладки грифа. Эта гитара имеет сквозную конструкцию с мензурой 25,5 дюймов, кленовым грифом, состоящим из трех частей, с накладкой из макассарского черного дерева и 24 ладами из нержавеющей стали экстра-джамбо. TE-1000 EverTune Koa обладает превосходным звуковым разнообразием благодаря набору активных звукоснимателей EMG 66TW (с push-pull) и EMG 57 (бридж).	6	Вклеенный гриф\n	648 мм (25.5")\n	Красное дерево с топом КОА\n	Клен, 3 части\n	Чёрное дерево\n	350	Natural Gloss\n	42	LTD locking\n	EMG 66TW\n	EMG 57\n	Активная	D'Addario XL120 (.009 - .042)\n	f		2	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_TE-1000_EVERTUNE_KOA_NATURAL_GLOSS(L).jpg
19	LTD J-430E NATURAL GLOSS электроакустическая гитара 6-струнная.\n\nПри своей доступной цене, электроакустические гитары LTD обладают многими характеристиками более дорогих представителей: великолепный стильный внешний вид с подчеркнутой текстурой древесины и многослойной окантовкой, фирменная эргономика инструментов ESP и достойное акустическое звучание.\n\n	6	Вклеенный гриф\n	25,5"/648 мм\n	Палисандр/Массив ситхинской ели\n	Чёрное дерево\n	Печённая ятоба\n	350	Natural Gloss\n	43	LTD	Fishman Sonicore\n	Fishman Sonicore\n	Fishman Presys I preamp w/ Onboard Tuner\n		f		1	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/acoustic/LTD_J-430E_NATURAL_GLOSS(L).jpg
16	LTD TA-334 BLK подписная 4-х струнная бас-гитара Tom Araya.\n\nТом Арайя (Tom Araya) - бессменный бас-гитарист, вокалист и сооснователь легендарной трэш-группы Slayer. Арайя родился в Чили, но его семья переехала в Калифорнию (США), когда ему было 5 лет. Том начал играть на басу еще ребенком - вместе со своим братом они играли и пели любимые песни. В 1981 году состоялось самое значительное событие для Тома - знакомство с Керри Кингом, который предложил ему участие в новой группе Slayer. Как музыкант, Арайя представляет собой жгучую смесь харизмы, мощного вокала и монументального звука баса.	4	Привинченный гриф\n	889 мм/35"\n	Липа	Клён	Палисандр	400	Black	40	LTD	ESP Designed ASB-4\n	ESP Designed ASB-4\n	Активная	ESP BS-30HB\n	f	Китай	2	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/bass/LTD_TA-334_BLK(L).jpg
5	LTD EC Arctic Metal Snow white satin электрогитара 6 струн, корпус красное дерево, гриф красное дерево/чёрное дерево, EMG 81TW.\n\nLTD Arctic Metal использует дизайн серии Black Metal, но ровно наоборот - вместо темнейшего чёрного эта серия использует холоднейший белый. Сравнимый по всем характеристикам с серие LTD 1000, Arctic Metal используют матовую белую отделку, один звукосниматель премиум-класса с белой крышкой и накладка на гриф из чёрного дерева без маркеров ладов и со светящимися в темноте боковыми маркерами. EC Arctic Metal основана на популярной форме ESP Eclipse. Она имеет глубоко вклеенный гриф с удобным доступом к верхним ладам, мензуру 24.75", агрессивный звукосниматель EMG 81TW с чёрным логотипом, одну ручку громкости с пуш-пуллом для отсечки катушки и 24 extra-jumbo лада из нержавеющей стали.	6	Глубоко вклеенный\n	628 мм (24.75")	Красное дерево	Красное дерево	Чёрное дерево	350	Snow White Satin	42	LTD Locking		EMG 81TW w/ Black Logo	Активная	D'Addario XL110 (.010 - .046)	f		1	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC_ARCTIC_METAL_SNOW_WHITE_SATIN(L).jpg
2	LTD M-1000 MULTI-SCALE See Thru Black Satin мультимензурная 6-струнная электрогитара.\n\nПрисоединяясь к расширяющемуся списку мультимензур, доступных в ESP, LTD M-1000 Multi-Scale представляет собой 6-струнную версию M-1007MS и M-1008MS. В мультимензурных гитарах используется угловое расположение ладов, чтобы обеспечить различную длину мензуры для различных струн гитары, что обеспечивает оптимальное натяжение струны, а также, как заявляют многие музыканты, для улучшения эргономики ладовой руки. M-1000 Multi-Scale имеет разную длину шкалы от 26,5 до 25,5 дюймов. Это дизайн с болтовым креплением с топом из огненного клена поверх задней деки из красного дерева и грифом из пяти частей, сделанным из клена и экзотического фиолетового дерева сердцевины, с накладкой из черного дерева Macassar со смещенными точками в виде морских ушек. Компоненты высшего уровня M-1000 Multi-Scale включают набор звукоснимателей Seymour Duncan Sentient (гриф) и Nazgul (бридж), которые были изготовлены на заказ для ESP со специальным углом, оптимизированным для подобной конструкции. Двухтактное управление на ручке тона позволяет разделить катушку, обеспечивая множество гибких тонов. Другие компоненты включают в себя многомензурный бридж Hipshot со струнами сквозь корпус и запирающие колки LTD. Инструмент выполнен в цвете See Thru Black Satin (прозрачный матовый чёрный).	6	Привинченный гриф	648 - 673 мм (25,5"- 26,5")	Красное дерево	Клен/Purple Heart, 5 частей	Черное дерево\n	350	See Thru Black Satin	42	LTD Локовые\n	Seymour Duncan Custom Sentient (наклонный)	Seymour Duncan Custom Nazgul (наклонный)	Пассивная	D'Addario XL110 (.010 - .046)	f		1	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-1000_MULTI-SCALE_SEE_THRU_BLACK_SATIN(L).jpg
\.


--
-- Data for Name: guitar_categories; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.guitar_categories (category_id, category_name, path_to_preview, path_to_photo) FROM stdin;
1	acoustic	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/acoustic(S).jpg	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/acoustic(L).jpg
2	bass	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/bass(S).jpg	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/bass(L).jpg
3	electro	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/electro(S).jpg	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/categories/electro(L).jpg
\.


--
-- Data for Name: guitars; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.guitars (guitar_id, category_id, article, name, price, path_to_preview, guitar_attr_id) FROM stdin;
1	3	LM1000MSFMSTBLKS	LTD M-1000 MULTI-SCALE SEE THRU BLACK SATIN	176483.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-1000_MULTI-SCALE_SEE_THRU_BLACK_SATIN(S).jpg	2
4	3	LECARMSWS	LTD EC ARCTIC METAL SNOW WHITE SATIN	114016.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC_ARCTIC_METAL_SNOW_WHITE_SATIN(S).jpg	5
5	3	LEC1000FRBLKS	LTD EC-1000FR BLKS	158877.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC-1000FR_BLKS(S).jpg	6
6	3	LEC1000PIEZOQMSTB	LTD EC-1000PIEZO QM STB	156655.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_EC-1000PIEZO_QM_STB(S).jpg	8
7	3	LHORIZONCTM87BLK	LTD HORIZON CUSTOM `87 BLK	201816.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_HORIZON_CUSTOM_`87_BLK(S).jpg	9
9	3	LM7BHTBKMBLKS	LTD M-7HT BARITONE BLACK METAL BLKS	122579.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-7HT_BARITONE_BLACK_METAL_BLKS(S).jpg	11
10	3	LM1000BPPRNB	LTD M-1000 PURPLE NATURAL BURST	164010.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_M-1000_PURPLE_NATURAL_BURST(S).jpg	12
13	3	LTE1000ETCHMS	LTD TE-1000 EVERTUNE CHARCOAL METALLIC SATIN	174380.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_TE-1000_EVERTUNE_CHARCOAL_METALLIC_SATIN(S).jpg	13
14	3	LTE1000ETKNAT	LTD TE-1000 EVERTUNE KOA NATURAL GLOSS	212344.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/LTD_TE-1000_EVERTUNE_KOA_NATURAL_GLOSS(S).jpg	14
15	3	LM200FMSTR	M-200 FM SEE THRU RED	69330.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/electro/M-200_FM_See_Thru_Red(S).jpg	15
17	2	LTA334BLK	LTD TA-334 BLK	56247.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/bass/LTD_TA-334_BLK(S).jpg	16
18	2	LB55FMSTRSB	LTD B-55 FM STRSB	33641.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/bass/LTD_B-55_FM_STRSB(S).jpg	17
19	1	LAD320ENS	LTD D-320E NATURAL SATIN	49628.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/acoustic/LTD_D-320E_NATURAL_SATIN(S).jpg	18
20	1	LAJ430ENAT	LTD J-430E NATURAL GLOSS	61375.00	/Users/semyonzhuravlev/Desktop/XcodeProjects/databaseProject/databaseProject/res/acoustic/LTD_J-430E_NATURAL_GLOSS(S).jpg	19
\.


--
-- Data for Name: order_items; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.order_items (order_item_id, guitar_id, order_id, quantity, user_id) FROM stdin;
30	18	2298	1	39
31	18	2299	1	39
32	10	2300	1	39
33	6	2300	1	39
34	4	2300	1	39
35	19	2301	2	39
36	20	2301	2	39
37	17	2302	1	39
38	19	2303	2	39
\.


--
-- Data for Name: orders; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.orders (order_id, user_id, order_date, shipping_address, status) FROM stdin;
2298	39	2024-12-11 14:03:30.60542	Москва                                                                                                                                                                                                                                                         	created
2299	39	2024-12-11 14:28:00.077808	МАИ                                                                                                                                                                                                                                                            	created
2300	39	2024-12-11 14:30:10.95976	америка                                                                                                                                                                                                                                                        	created
2301	39	2024-12-11 14:37:00.658593	Гусь-Хрустальный                                                                                                                                                                                                                                               	created
2302	39	2024-12-12 06:03:52.424976	Москва                                                                                                                                                                                                                                                         	created
2303	39	2024-12-12 06:45:24.223265	ЛОНДОН                                                                                                                                                                                                                                                         	created
\.


--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.users (user_id, type, login, password, email, phone_number, name) FROM stdin;
1	admin	admin	8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918	admin@mail.ru	+79006666666	Admin
39	user	zhuravlevsema	25468573ab1e25997805f9165d2a677bcaccedcafe3abdf387f3d600b3e0458e	mail@mail.ru	+79307412407	Semyon Zhuravlev
40	user	123A	2dd1a5a6d2bd5eded5c7c317ecb96c18cd7efa598e0231921bbabe6e95574f51	ME@mail.ru	+79301111111	SEMEN
41	user	vasya	a1f4fd39ae9a17a978232327521aa3761d5aa10cda443d00532909fd5230f43d	mmail@mail.ru	+79301234567	Семен Журавлев
\.


--
-- Name: cart_items_cart_item_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cart_items_cart_item_id_seq', 73, true);


--
-- Name: guitar_attributes_guitar_attr_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.guitar_attributes_guitar_attr_id_seq', 38, true);


--
-- Name: guitar_categories_category_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.guitar_categories_category_id_seq', 3, true);


--
-- Name: guitars_guitar_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.guitars_guitar_id_seq', 35, true);


--
-- Name: order_items_order_item_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.order_items_order_item_id_seq', 38, true);


--
-- Name: orders_order_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.orders_order_id_seq', 2303, true);


--
-- Name: users_user_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.users_user_id_seq', 41, true);


--
-- Name: cart_items cart_items_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cart_items
    ADD CONSTRAINT cart_items_pkey PRIMARY KEY (cart_item_id);


--
-- Name: guitar_attributes guitar_attributes_path_to_photo_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitar_attributes
    ADD CONSTRAINT guitar_attributes_path_to_photo_unique UNIQUE (path_to_photo);


--
-- Name: guitar_attributes guitar_attributes_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitar_attributes
    ADD CONSTRAINT guitar_attributes_pkey PRIMARY KEY (guitar_attr_id);


--
-- Name: guitar_categories guitar_categories_path_to_photo_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitar_categories
    ADD CONSTRAINT guitar_categories_path_to_photo_unique UNIQUE (path_to_photo);


--
-- Name: guitar_categories guitar_categories_path_to_preview_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitar_categories
    ADD CONSTRAINT guitar_categories_path_to_preview_unique UNIQUE (path_to_preview);


--
-- Name: guitar_categories guitar_categories_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitar_categories
    ADD CONSTRAINT guitar_categories_pkey PRIMARY KEY (category_id);


--
-- Name: guitars guitars_article_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_article_unique UNIQUE (article);


--
-- Name: guitars guitars_name_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_name_unique UNIQUE (name);


--
-- Name: guitars guitars_path_to_preview_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_path_to_preview_unique UNIQUE (path_to_preview);


--
-- Name: guitars guitars_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_pkey PRIMARY KEY (guitar_id);


--
-- Name: order_items order_items_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.order_items
    ADD CONSTRAINT order_items_pkey PRIMARY KEY (order_item_id);


--
-- Name: orders orders_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.orders
    ADD CONSTRAINT orders_pkey PRIMARY KEY (order_id);


--
-- Name: users users_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_email_key UNIQUE (email);


--
-- Name: users users_login_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_login_key UNIQUE (login);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (user_id);


--
-- Name: order_items after_item_added; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER after_item_added AFTER INSERT ON public.order_items FOR EACH ROW EXECUTE FUNCTION public.process_order_items();


--
-- Name: guitars delete_guitar_attributes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER delete_guitar_attributes AFTER DELETE ON public.guitars FOR EACH ROW EXECUTE FUNCTION public.delete_guitar_attributes_func();


--
-- Name: cart_items cart_items_guitar_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cart_items
    ADD CONSTRAINT cart_items_guitar_id_foreign FOREIGN KEY (guitar_id) REFERENCES public.guitars(guitar_id) ON DELETE CASCADE;


--
-- Name: cart_items cart_items_user_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cart_items
    ADD CONSTRAINT cart_items_user_id_foreign FOREIGN KEY (user_id) REFERENCES public.users(user_id);


--
-- Name: guitars guitars_category_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_category_foreign FOREIGN KEY (category_id) REFERENCES public.guitar_categories(category_id);


--
-- Name: guitars guitars_category_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.guitars
    ADD CONSTRAINT guitars_category_id_foreign FOREIGN KEY (category_id) REFERENCES public.guitar_categories(category_id);


--
-- Name: order_items order_items_guitar_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.order_items
    ADD CONSTRAINT order_items_guitar_id_foreign FOREIGN KEY (guitar_id) REFERENCES public.guitars(guitar_id) ON DELETE CASCADE;


--
-- Name: order_items order_items_order_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.order_items
    ADD CONSTRAINT order_items_order_id_foreign FOREIGN KEY (order_id) REFERENCES public.orders(order_id);


--
-- Name: orders orders_user_id_foreign; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.orders
    ADD CONSTRAINT orders_user_id_foreign FOREIGN KEY (user_id) REFERENCES public.users(user_id);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: pg_database_owner
--

GRANT USAGE ON SCHEMA public TO basic_user;
GRANT USAGE ON SCHEMA public TO read_only;


--
-- Name: TABLE cart_items; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.cart_items TO basic_user;
GRANT SELECT ON TABLE public.cart_items TO read_only;


--
-- Name: TABLE guitar_attributes; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.guitar_attributes TO basic_user;
GRANT SELECT ON TABLE public.guitar_attributes TO read_only;


--
-- Name: TABLE guitar_categories; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.guitar_categories TO basic_user;
GRANT SELECT ON TABLE public.guitar_categories TO read_only;


--
-- Name: TABLE guitars; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.guitars TO basic_user;
GRANT SELECT ON TABLE public.guitars TO read_only;


--
-- Name: TABLE guitar_details_view; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.guitar_details_view TO basic_user;
GRANT SELECT ON TABLE public.guitar_details_view TO read_only;


--
-- Name: TABLE order_items; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.order_items TO basic_user;
GRANT SELECT ON TABLE public.order_items TO read_only;


--
-- Name: TABLE orders; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.orders TO basic_user;
GRANT SELECT ON TABLE public.orders TO read_only;


--
-- Name: TABLE users; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE public.users TO basic_user;
GRANT SELECT ON TABLE public.users TO read_only;


--
-- Name: DEFAULT PRIVILEGES FOR TABLES; Type: DEFAULT ACL; Schema: public; Owner: admin
--

ALTER DEFAULT PRIVILEGES FOR ROLE admin IN SCHEMA public GRANT SELECT ON TABLES TO basic_user;
ALTER DEFAULT PRIVILEGES FOR ROLE admin IN SCHEMA public GRANT SELECT ON TABLES TO read_only;


--
-- PostgreSQL database dump complete
--

